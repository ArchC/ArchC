#!/bin/bash

# NightlyTester script for ArchC.
# Clone ArchC source in GIT and tests current version
#
# ArchC Team

# Parameters adjustable by environment variables

NIGHTLYVERSION=2.0

usage(){
		echo -ne "\nusage: ./nightlytester.sh <config_file>\n\n"
}

if [ $# -eq 0 ]; then
    usage
    exit
else
case "$1" in
    -h)
        usage
        exit
        ;;

    *   )
        if ! [ -f "$1" ]; then
           echo -ne "\nConfiguration file not found. Must run with a valid configuration file.\n\n"
           exit 1
        fi
       ;;
esac
fi

. $1

# Configuring ACSIM param
if [ "$COLLECT_STATS" != "no" ]; then
  ACSIM_PARAMS="${ACSIM_PARAMS} --stats"
fi

# ********************************
# * Software location constants **
# ********************************
# If user-supplied path is not present, we must compile our own software packages

# SystemC
if [ -z "$SYSTEMCPATH" ]; then
  SYSTEMCPATH=${TESTROOT}/systemc/install
  SYSTEMCCOMPILE=yes
else
  SYSTEMCCOMPILE=no
fi

# *************************************
# * Extracted sources path constants **
# *************************************


# Functions


is_spec2006_enabled(){
    if  [ "$BZIP_2" == "no" ] &&
        [ $MCF == "no" ] &&
        [ $GOBMK == "no" ] &&
        [ $HMMER == "no" ] &&      
        [ $SJENG == "no" ] &&
        [ $LIBQUANTUM == "no" ] &&
        [ $H264 == "no" ] &&        
        [ $OMNETPP == "no" ] &&
        [ $ASTAR == "no" ]; then
            return 1;
        else
            return 0;
        fi
}


finalize_nightly_tester() {
  TEMPFL=${RANDOM}.out

  DATE_TMP=`LANG=en_US date '+%a %D %T'`
  sed -n -e  "s@Produced by NightlyTester.*@Produced by NightlyTester \@ $DATE_TMP <\/p>@;p" < ${HTMLINDEX} > TMPFILE
  mv TMPFILE ${HTMLINDEX}
  rm -f TMPFILE


  if [ "$LASTEQCURRENT" != "true" ]; then
      HTMLLINE="<tr><td>${HTMLPREFIX}</td><td>${DATE}</td><td>${ARCHCREV}</td><td><a href=\"${HTMLPREFIX}-index.htm\">Here</a></td><td>${REVMESSAGE}</td><td>${HOSTNAME}</td></tr>"
      sed -e "/<tr><td>${LASTHTMLPREFIX}/i${HTMLLINE}" $HTMLINDEX > $TEMPFL
      mv ${TEMPFL} $HTMLINDEX
  fi

  if [ "$DELETEWHENDONE" != "no" ]; then
    rm -rf $TESTROOT
  else
    echo -ne "${TESTROOT} folder with all the tests won't be deleted because \$DELETEWHENDONE is set to \"no\".\n"
  fi
}

do_abort() {
  echo -ne "Aborting...\n\n"
  finalize_nightly_tester
  exit 1
}

#
# Below some useful HTML formating functions from validation.sh
#

# Param1 is the HTML file name
# Param2 is the "title" string
initialize_html() {
	echo -ne "<html> <head> <title> ${2} </title> </head> <body>" > $1
	echo -ne "<h1>${2}</h1>" >> $1
}

# Param1 is the HTML file name
# Param2 is an string containing extra tag terminations (e.g. "</table>")
finalize_html() {
	echo -ne "${2}</body>" >> ${1}
}

# Param1 is the input raw text to format
# Param2 is the HTML file name
format_html_output() {
	echo -ne "<table><tr><td><font face=\"Courier\">" >>$2
	sed -e 'a\<br\>' -e 's/ /\&nbsp;/g' -e 's/error/\<b\>\<font color=\"crimson\"\>error\<\/font\>\<\/b\>/' -e 's/warning/\<b\>\<font color=\"fuchsia\"\>warning\<\/font\>\<\/b\>/' <$1 1>>$2 2>/dev/null
	echo -ne "</font></td></tr></table>"  >>$2
}

# This function is used to get source model, in GIT repository or local Working Copy
clone_or_copy_model(){
  MODELNAME=$1
  GITLINK=$2
  MODELWORKINGCOPY=$3

  mkdir -p ${TESTROOT}/${MODELNAME}/base
  if [ -z "$GITLINK" ]; then
    echo -ne "Copying $MODELNAME source from a local directory...\n"
    cp -a ${MODELWORKINGCOPY} ${TESTROOT}/${MODELNAME}/base &> /dev/null
    [ $? -ne 0 ] && {
      echo -ne "<p><b><font color=\"crimson\">${MODELNAME} source copy failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
      finalize_html $HTMLLOG ""
      echo -ne "Local directory copy \e[31mfailed\e[m. Check script parameters.\n"
      do_abort
    }
    cd ${TESTROOT}/${MODELNAME}/base
    MODELREV="N/A"
    cd - &> /dev/null
  else
    echo -ne "Cloning ${MODELNAME} ArchC Model GIT...\n"
    TEMPFL=${RANDOM}.out
    #svn co ${SVNLINK} ${TESTROOT}/${MODELNAME} > $TEMPFL 2>&1
    git clone ${GITLINK} ${TESTROOT}/${MODELNAME}/base > $TEMPFL 2>&1
    [ $? -ne 0 ] && {
      rm $TEMPFL
      echo -ne "<p><b><font color=\"crimson\">${modelname} model git clone failed. check script parameters.</font></b></p>\n" >> $htmllog
      finalize_html $htmllog ""
      echo -ne "git clone \e[31mfailed\e[m. check script parameters.\n"
      do_abort
    } 
    # Extract model revision number
    cd ${TESTROOT}/${MODELNAME}/base
    #MODELREV=$(git log | head -n1 | cut -c8-13)"..."$(git log | head -n1 | cut -c42-)
    MODELREV=$(git log | head -n1 | cut -c8-15)".."
    if [ "$LASTHTMLPREFIX" != "0" ]; then
        LASTMODELREV=`grep -e "<td>$MODELNAME" < ${LOGROOT}/${LASTHTMLPREFIX}-index.htm | head -n 1 | cut -d\> -f 5 | cut -d\< -f 1`
        if [ "$MODELREV" != "$LASTMODELREV" ]; then
            LASTEQCURRENT="false"
        fi
    else
        LASTEQCURRENT="false"
    fi
    cd - &> /dev/null
    #MODELREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
    rm $TEMPFL
  fi

}

build_model() {
      MODELNAME=$1
      USEACSIM=$2
      LOCAL_PARAMS=$3  # Each test have a specific set of params
      LOCAL_DIR=$4     # Each test have a specific dir (e.g. arm/acsim, arm/accsim, arm/acstone)
    
      build_fault="no"  # funcion return

    if [ "$USEACSIM" != "no" ]; then    
        echo -ne "Building ${MODELNAME} ArchC Model with [ ${LOCAL_PARAMS} ] params...\n"
        cd ${TESTROOT}/${MODELNAME}
        cp -r base $LOCAL_DIR
        cd $LOCAL_DIR
        TEMPFL=${RANDOM}.out
        if [ $LOCALSIMULATOR != "no" ]; then
            echo -ne "Using local source simulator...\n"
            if [ -e Makefile.archc ]; then
                make -f Makefile.archc clean &> /dev/null
                make -f Makefile.archc >> $TEMPFL 2>&1
            else
                echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model GIT can not be used with LOCALSIMULATOR=yes. Check script parameters.</font></b></p>\n" >> $HTMLLOG
                finalize_html $HTMLLOG ""
                echo -ne "GIT clone \e[31mfailed\e[m. GIT can not be used with LOCALSIMULATOR=yes. Check script parameters.\n"
                do_abort
           fi 
        else
            if [ -e Makefile.archc ]; then
                make -f Makefile.archc distclean &> /dev/null
            fi
            ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac ${LOCAL_PARAMS} > $TEMPFL 2>&1 && make -f Makefile.archc >> $TEMPFL 2>&1  
        fi
        RETCODE=$?
        HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-${LOCAL_DIR}-build-log.htm
        initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV build output"
        format_html_output $TEMPFL $HTMLBUILDLOG
        finalize_html $HTMLBUILDLOG ""
        rm $TEMPFL
        if [ $RETCODE -ne 0 ]; then
            echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV build failed. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-${LOCAL_DIR}-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
            finalize_html $HTMLLOG ""
            echo -ne "ACSIM \e[31mfailed\e[m to build $MODELNAME model.\n"
            build_fault="yes"
        else
            echo -ne "<p>${MODELNAME} Model rev. $MODELREV built successfully. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-${LOCAL_DIR}-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
        fi
    fi
}

build_model_gdb () {
    MODELNAME=$1
    USEACSIM=$2
      
    if [ "$USEACSIM" != "no" ]; then    
        if [ "$RUN_ACSTONE" != "no" ]; then
            echo -ne "Building ${MODELNAME} ArchC Model with gdb support for acstone...\n"
            cd ${TESTROOT}/${MODELNAME}
            cp -r base acstone
            cd acstone
            TEMPFL=${RANDOM}.out
            ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac ${ACSIM_PARAMS} -gdb > $TEMPFL 2>&1 && make -f Makefile.archc >> $TEMPFL 2>&1
            RETCODE=$?
            HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-build-acstone-log.htm
            initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV build output (with gdb support for acstone use)"
            format_html_output $TEMPFL $HTMLBUILDLOG
            finalize_html $HTMLBUILDLOG ""
            rm $TEMPFL
            if [ $RETCODE -ne 0 ]; then
                echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV build with gdb support (for acstone) failed. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-acstone-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
                finalize_html $HTMLLOG ""
                echo -ne "ACSIM (gdb) \e[31mfailed\e[m to build $MODELNAME model.\n"
                do_abort
            else
                echo -ne "<p>${MODELNAME} Model rev. $MODELREV with gdb support (for acstone) built successfully. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-acstone-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
            fi
        fi
    fi
}

# This function is used to build binutils tools for a given model, testing ACASM, ArchC's binary tools synthetizer.
build_binary_tools() {
  MODELNAME=$1
  cd ${TESTROOT}/${MODELNAME}
  mkdir -p binutils
  echo -ne "Building ${MODELNAME} BINUTILS ArchC Model...\n"
  TEMPFL=${RANDOM}.out
  ${TESTROOT}/install/bin/acbingen.sh -a ${MODELNAME}_1 -f ${MODELNAME}.ac > $TEMPFL 2>&1 &&
    mkdir build-binutils &&
    cd build-binutils && # D_FORTIFY used below is used to prevent a bug present in binutils 2.15 and 2.16
    #CFLAGS="-w -g -O2 -D_FORTIFY_SOURCE=1" ${BINUTILSPATH}/configure --target=${MODELNAME}_1-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
    CFLAGS="-w -g -O2" ${BINUTILSPATH}/configure --target=${MODELNAME}_1-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1 
  RETCODE=$?
  HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-binutils-build-log.htm
  initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV acbinutils build output"
  format_html_output $TEMPFL $HTMLBUILDLOG
  finalize_html $HTMLBUILDLOG ""
  rm $TEMPFL
  if [ $RETCODE -ne 0 ]; then
    echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV binutils build failed (using acbingen). Check <a href=\"${HTMLPREFIX}-${MODELNAME}-binutils-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
    finalize_html $HTMLLOG ""
    echo -ne "ACASM \e[31mfailed\e[m to build $MODELNAME binutils tools.\n"
    do_abort
  else
    echo -ne "<p>${MODELNAME} binutils built successfully using acbingen. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-binutils-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
  fi  
}

# This function is used to build gdb for a given model, testing acstone, which uses gdb for validation.
build_gdb() {
  MODELNAME=$1
  cd ${TESTROOT}/${MODELNAME}
  mkdir -p binutils
  echo -ne "Building GDB for ${MODELNAME}...\n"
  TEMPFL=${RANDOM}.out
  #${TESTROOT}/install/bin/acbingen.sh -a ${MODELNAME}_1 -f ${MODELNAME}.ac > $TEMPFL 2>&1 &&    
    mkdir build-gdb &&
    cd build-gdb &&
    ${GDBPATH}/configure --target=${MODELNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1
  RETCODE=$?
  HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-gdb-build-log.htm
  initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV gdb build output"
  format_html_output $TEMPFL $HTMLBUILDLOG
  finalize_html $HTMLBUILDLOG ""
  rm $TEMPFL
  if [ $RETCODE -ne 0 ]; then
    echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV gdb build failed (using acbingen). Check <a href=\"${HTMLPREFIX}-${MODELNAME}-gdb-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
    finalize_html $HTMLLOG ""
    echo -ne "ACASM \e[31mfailed\e[m to build $MODELNAME gdb.\n"
    do_abort
  else
    echo -ne "<p>${MODELNAME} gdb built successfully using acbingen. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-gdb-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
  fi  
}

# This function is used to build binutils *ORIGINAL* tools for a given architecture, so we can compare and expect the results from these tools to be correct and
# validate ArchC's generated ones.
build_original_toolchain() {
  MODELNAME=$1
  ARCHNAME=$2
  cd ${TESTROOT}/${MODELNAME}
  mkdir binutils-orig
  mkdir build-binutils-orig
  echo -ne "Building ${ARCHNAME} toolchain for reference...\n"
  echo -ne "  Building binutils...\n"
  TEMPFL=${RANDOM}.out
  cd build-binutils-orig &&
    CFLAGS="-w -O2" ${BINUTILSPATH}/configure --target=${ARCHNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils-orig >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1    
  RETCODE=$?
  HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-binutils-original-build-log.htm
  initialize_html $HTMLBUILDLOG "${ARCHNAME} binutils build output"
  format_html_output $TEMPFL $HTMLBUILDLOG
  finalize_html $HTMLBUILDLOG ""
  rm $TEMPFL
  if [ $RETCODE -ne 0 ]; then
    echo -ne "<p><b><font color=\"crimson\">${ARCHNAME} binutils build failed. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-binutils-original-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
    finalize_html $HTMLLOG ""
    echo -ne "${ARCHNAME} binutils build failed.\n"
    do_abort
  else
    echo -ne "<p>${ARCHNAME} original binutils built successfully. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-binutils-original-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
  fi
}

# This functions is used to run simulations tests using ACSTONE and ArchC's generated simulator for a target architecture
run_tests_acsim_acstone() {
  MODELNAME=$1
  MODELREV=$2

  ### WORKAROUND: In armv5e model, we must rename it to "arm", see "build_model()" for more info  
  if [ "$MODELNAME" = "arm" ]; then
    MODELNAME="arm"     
  fi
  ### WORKAROUND: In sparcv8 model, we must rename it to "sparc", see "build_model()" for more info  
  if [ "$MODELNAME" = "sparc" ]; then
    MODELNAME="sparc"     
  fi
  ### WORKAROUND: In powerpc model, we must rename it to "powerpc", see "build_model()" for more info
  if [ "$MODELNAME" = "powerpc" ]; then
    MODELNAME="powerpc" 
  fi

  export TESTER=${TESTROOT}/acstone/acstone_run_teste.sh
  export SOMADOR=${TESTROOT}/acstone/collect_stats.py
  export SIMULATOR="${TESTROOT}/${MODELNAME}/acstone/${MODELNAME}.x"
  export STATS=${TESTROOT}/acstone/${MODELNAME}.stats.txt
  export GDBEXEC=${TESTROOT}/${MODELNAME}/binutils/bin/${MODELNAME}-elf-gdb
  export LOGROOT
  export HTMLPREFIX

  cd ${TESTROOT}/acstone
  ./acstone_run_all.sh $MODELNAME

  echo -ne "<tr><td>${MODELNAME} (acstone)</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}-acstone.htm\">Here</a></td></tr>\n" >> $HTMLLOG
}

# This function runs ACASM validation script, assembling and linking a series of .s files using both the generated assembler and a correct reference
# assembler (binutils for a given architecture). The results are compared between these two assemblers and validation is successful if they have no
# difference.
run_tests_acasm() {
  MODELNAME=$1
  ARCHNAME=$2
  if [ "$MODELNAME" = "arm" ]; then
    cd ${TESTROOT}/acasm-validation/arm/runtest
    export BENCH_ROOT="${TESTROOT}/acasm-validation/arm/benchmark/Mibench"
  else
    cd ${TESTROOT}/acasm-validation/${MODELNAME}/runtest
    export BENCH_ROOT="${TESTROOT}/acasm-validation/${MODELNAME}/benchmark/Mibench"
  fi
  export ACBIN_PATH="${TESTROOT}/${MODELNAME}/binutils/${MODELNAME}_1-elf/bin"
  export BINUTILS_PATH="${TESTROOT}/${MODELNAME}/binutils-orig/${ARCHNAME}-elf/bin"
  LOG_FILE=l${RANDOM}.out
  FORM_FILE=f${RANDOM}.out
  HTML_LOG_FILE=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-report.htm
  export LOG_FILE
  export FORM_FILE
  export HTML_LOG_FILE
  #../../runtest.sh --verbose-log ../mibench.conf > /dev/null 2>&1
  ../../runtest.sh --verbose-log ../mibench.conf 
  HTMLACASMLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-log.htm
  HTMLACASMFORM=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-form.htm
  format_html_output $LOG_FILE $HTMLACASMLOG
  format_html_output $FORM_FILE $HTMLACASMFORM
  echo -ne "<tr><td>${MODELNAME}</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-report.htm\">Report</a>, <a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-log.htm\">Log</a>, <a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-form.htm\">Form</a></td></tr>\n" >> $HTMLLOG
}

run_tests_acsim() {
  MODELNAME=$1
  MODELBENCHROOT=$2
  MODELSPECROOT=$3
  MODELREV=$4
  DIRSIMULATOR=$5  # Each test have simulators with a specific set of params (e.g. arm/acsim, arm/acstone, arm/powersc)


  # Preparing test script
  ARCH="${MODELNAME}"
  SIMULATOR="${TESTROOT}/${MODELNAME}/$DIRSIMULATOR/${MODELNAME}.x --load="
  GOLDENROOT=${TESTROOT}/acsim/GoldenMibench
  GOLDENSPECROOT=${TESTROOT}/acsim/GoldenSpec
  MIBENCHROOT=${MODELBENCHROOT} 
  SPECROOT=${MODELSPECROOT} 
  STATSROOT=${BENCHROOT}/stats
  # Collect statistical information 
  if [ "$COLLECT_STATS" != "no" ]; then
    mkdir -p ${STATSROOT}
    cp ${SCRIPTROOT}/collect_stats.py ${STATSROOT}
  fi
  export ARCH
  export DIRSIMULATOR
  export SIMULATOR 
  export RUNSMALL   # ==================================
  export RUNLARGE   # Definition in nightlytester.conf
  export COMPILE    # ==================================
  export GOLDENROOT
  export GOLDENSPECROOT
  export MIBENCHROOT
  export SPECROOT
  export STATSROOT
  export COLLECT_STATS
  export RUN_POWERSC

  # Define which programs to test (definition in nightlytester.conf)  
  export BASICMATH
  export BITCOUNT
  export QUICKSORT
  export SUSAN
  export ADPCM
  export CRC
  export FFT
  export GSM
  export DIJKSTRA
  export PATRICIA
  export RIJNDAEL
  export SHA
  export JPEG
  export LAME

  export BZIP_2	
  export MCF 
  export GOBMK    
  export HMMER      
  export SJENG 	    
  export LIBQUANTUM
  export H264        
  export OMNETPP    
  export ASTAR      

  export TESTROOT
  export TESTCOMPILER
  export TESTCOMPILERCPP
  export TESTAR
  export TESTRANLIB
  export TESTFLAG

  export -f is_spec2006_enabled

  cd ${TESTROOT}/acsim
  ./validation.sh
  
  echo -ne "<tr><td>${MODELNAME} </td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}.htm\">Here</a></td></tr>\n" >> $HTMLLOG
}

# This function is used to run simulation tests using Mibench and ArchC's generated *COMPILED* simulator for a target architecture
run_tests_accsim_mibench() {
  MODELNAME=$1
  MODELBENCHROOT=$2
  MODELREV=$3
  ACCSIMFLAGS=$4" "${ACCSIM_PARAMS}

  # Temporary - not supporting stats collection for accsim
  COLLECT_STATS=no

  # Preparing test script
  ARCH="${MODELNAME}"
  SIMULATOR="${TESTROOT}/${MODELNAME}/${MODELNAME}.x"
  GOLDENROOT=${TESTROOT}/acsim/GoldenMibench
  BENCHROOT=${MODELBENCHROOT}  
  STATSROOT=${BENCHROOT}/stats
  SIMROOT=${TESTROOT}/${MODELNAME}
  TOOLSPATH=${TESTROOT}/install/bin

  # Collect statistical information 
  if [ "$COLLECT_STATS" != "no" ]; then
    mkdir -p ${STATSROOT}
    cp ${SCRIPTROOT}/collect_stats.py ${STATSROOT}
  fi
  export ARCH
  export SIMULATOR 
  export RUNSMALL   # ==================================
  export RUNLARGE   # Definition in nightlytester.conf
  export COMPILE    # ==================================
  export GOLDENROOT
  export BENCHROOT
  export STATSROOT
  export COLLECT_STATS
  export SIMROOT
  export TOOLSPATH
  export ACCSIMFLAGS

  # Define which programs to test (definition in nightlytester.conf)  
  export BASICMATH
  export BITCOUNT
  export QUICKSORT
  export SUSAN
  export ADPCM
  export CRC
  export FFT
  export GSM
  export DIJKSTRA
  export PATRICIA
  export RIJNDAEL
  export SHA
  export JPEG
  export LAME

  cd ${TESTROOT}/acsim
  ./validation-accsim.sh
  
  echo -ne "<tr><td>${MODELNAME} (mibench)</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}-accsim.htm\">Here</a></td></tr>\n" >> $HTMLLOG
}


#######################################
### TEST DRIVER
#######################################

test_acsim_simple() {

    echo -ne "<h3>Testing: ACSIM </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build ACSIM models: <b> ./acsim model.ac ${ACSIM_PARAMS} </b> </p>\n" >> $HTMLLOG

    echo -ne "\n**********************************************\n"
    echo -ne "* Testing ACSIM simple                      **\n"
    echo -ne "**********************************************\n"

    if [ "$RUN_ARM_ACSIM" != "no" ]; then
        build_model "arm" "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS}" "acsim" 
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
        build_model "sparc" "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS}" "acsim"
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
        build_model "mips" "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS}" "acsim"
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
        build_model "powerpc" "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS}" "acsim"
    fi

    cp ${SCRIPTROOT}/validation.sh ${TESTROOT}/acsim/validation.sh
    chmod u+x ${TESTROOT}/acsim/validation.sh
    export LOGROOT
    export HTMLPREFIX
    
    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG

    if [ "$RUN_ARM_ACSIM" != "no" ]; then
        echo -ne "\n Running ARM... \n"
        run_tests_acsim "arm" "${TESTROOT}/acsim/ARMMibench" "${TESTROOT}/acsim/ARMSpec" "${ARMREV}" "acsim" 
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
        echo -ne "\n Running Sparc... \n"
        run_tests_acsim "sparc" "${TESTROOT}/acsim/SparcMibench" "${TESTROOT}/acsim/SparcSpec" "${SPARCREV}" "acsim" 
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
        echo -ne "\n Running Mips... \n"
        run_tests_acsim "mips" "${TESTROOT}/acsim/MipsMibench" "${TESTROOT}/acsim/MipsSpec" "${MIPSREV}" "acsim" 
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
        echo -ne "\n Running PowerPC... \n"
        run_tests_acsim "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${TESTROOT}/acsim/PowerPCSpec" "${PPCREV}" "acsim" 
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

test_accsim() {

    echo -ne "<h3>Testing: ACCSIM </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build ACCSIM models: <b> ./accsim model.ac ${ACCSIM_PARAMS} /path/to/program </b> </p>\n" >> $HTMLLOG

    echo -ne "\n**********************************************\n"
    echo -ne "* Testing ACCSIM simple                      **\n"
    echo -ne "**********************************************\n"

    cp ${SCRIPTROOT}/validation-accsim.sh ${TESTROOT}/acsim/validation-accsim.sh
    chmod u+x ${TESTROOT}/acsim/validation-accsim.sh
    export LOGROOT
    export HTMLPREFIX

    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG

    if [ "$RUN_ARM_ACCSIM" != "no" ]; then  
      run_tests_accsim_mibench "arm" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}" "-ai"
    fi
    if [ "$RUN_SPARC_ACCSIM" != "no" ]; then  
      run_tests_accsim_mibench "sparc" "${TESTROOT}/acsim/SparcMibench" "${SPARCREV}" "-opt 2"
    fi
    if [ "$RUN_MIPS_ACCSIM" != "no" ]; then  
      run_tests_accsim_mibench "mips" "${TESTROOT}/acsim/MipsMibench" "${MIPSREV}" "-opt 2"
    fi
    if [ "$RUN_POWERPC_ACCSIM" != "no" ]; then  
      run_tests_accsim_mibench "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${PPCREV}" "-opt 2"
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

test_acasm() {

    echo -ne "<h3>Testing: ACASM </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build ACASM: <b> acbingen.sh -a model -f model.ac </b> </p>\n" >> $HTMLLOG
    
    echo -ne "\n**********************************************\n"
    echo -ne "* Testing acasm                             **\n"
    echo -ne "**********************************************\n"

    if [ "$RUN_ARM_ACASM" != "no" ]; then
      build_binary_tools "arm"
      build_original_toolchain "arm" "arm"
    fi
    if [ "$RUN_SPARC_ACASM" != "no" ]; then
      build_binary_tools "sparc"
      build_original_toolchain "sparc" "sparc"
    fi
    if [ "$RUN_MIPS_ACASM" != "no" ]; then
      build_binary_tools "mips"
      build_original_toolchain "mips" "mips"
    fi
    if [ "$RUN_POWERPC_ACASM" != "no" ]; then
      build_binary_tools "powerpc"
      build_original_toolchain "powerpc" "powerpc"
    fi
 
    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG

    echo -ne "Uncompressing ACASM validation package...\n"
    cd $TESTROOT
    tar -xjf ${SCRIPTROOT}/sources/acasm-validation.tar.bz2
    [ $? -ne 0 ] && do_abort
    chmod u+x acasm-validation/runtest.sh
    
    if [ "$RUN_ARM_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for arm ArchC model...\n"
      run_tests_acasm "arm" "arm"
    fi
    if [ "$RUN_MIPS_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for mips ArchC model...\n"
      run_tests_acasm "mips" "mips"
    fi
    if [ "$RUN_SPARC_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for sparc ArchC model...\n"
      run_tests_acasm "sparc" "sparc"
    fi
    if [ "$RUN_POWERPC_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for powerpc ArchC model...\n"
      run_tests_acasm "powerpc" "powerpc"
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

test_acstone() {

    echo -ne "<h3>Testing: ACSTONE </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to ACSTONE: <b> ./acsim model.ac ${ACSIM_PARAMS} -gdb </b> </p>\n" >> $HTMLLOG

    echo -ne "\n**********************************************\n"
    echo -ne "* Testing ACSTONE                           **\n"
    echo -ne "**********************************************\n"


    # Extracts acstone binaries (arm, mips, powerpc and sparc) as well as the helper scripts
    cd ${TESTROOT} &> /dev/null
    tar -xjf ${SCRIPTROOT}/sources/AllArchs-acstone.tar.bz2
    [ $? -ne 0 ] && do_abort
    cp ${SCRIPTROOT}/acstone_run_all.sh ${TESTROOT}/acstone &&
      cp ${SCRIPTROOT}/acstone_run_teste.sh ${TESTROOT}/acstone &&
      cp ${SCRIPTROOT}/collect_stats.py ${TESTROOT}/acstone
    [ $? -ne 0 ] && do_abort	
    chmod u+x ${TESTROOT}/acstone/*.sh
    [ $? -ne 0 ] && do_abort	
    # Fix arm names
    cd ${TESTROOT}/acstone &> /dev/null
    #  for MODELFILE in `find *arm*`
    #    do
    #	NEWFILENAME=`sed -e 's/arm/arm/' <<< "$MODELFILE"`
    #	cp $MODELFILE $NEWFILENAME
    #    done
    cd - &> /dev/null

    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG

    if [ "$RUN_ARM_ACSIM" != "no" ]; then
      build_model "arm" "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "arm"
      run_tests_acsim_acstone "arm" "${ARMREV}"
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
      build_model "sparc" "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "sparc"
      run_tests_acsim_acstone "sparc" "${SPARCREV}"
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
      build_model "mips" "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "mips"
      run_tests_acsim_acstone "mips" "${MIPSREV}"
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
      build_model "powerpc" "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "powerpc"
      run_tests_acsim_acstone "powerpc" "${PPCREV}"    
    fi

    finalize_html $HTMLLOG "</table></p>"
}

test_powersc() {

    echo -ne "<h3>Testing: PowerSC </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build PowerSC models: <b> ./acsim model.ac ${ACSIM_PARAMS} -pw </b> </p>\n" >> $HTMLLOG

    echo -ne "\n**********************************************\n"
    echo -ne "* Testing PowerSC                           **\n"
    echo -ne "**********************************************\n"

    ARM_POWERSC=$RUN_ARM_ACSIM
    SPARC_POWERSC=$RUN_SPARC_ACSIM
    MIPS_POWERSC=$RUN_MIPS_ACSIM
    POWERPC_POWERSC=$RUN_POWERPC_ACSIM


    if [ "$RUN_ARM_ACSIM" != "no" ]; then
        build_model "arm" "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        [[ "$build_fault" == "yes" ]] && ARM_POWERSC="no"
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
        build_model "sparc" "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        [[ "$build_fault" == "yes" ]] && SPARC_POWERSC="no"
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
        build_model "mips" "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        [[ "$build_fault" == "yes" ]] && MIPS_POWERSC="no"
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
        build_model "powerpc" "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        [[ "$build_fault" == "yes" ]] && POWERPC_POWERSC="no"
    fi

    cp ${SCRIPTROOT}/validation.sh ${TESTROOT}/acsim/validation.sh
    chmod u+x ${TESTROOT}/acsim/validation.sh
    export LOGROOT
    export HTMLPREFIX
    
    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG

    if [ "$ARM_POWERSC" != "no" ]; then
        echo -ne "\n Running ARM... \n"
        run_tests_acsim "arm" "${TESTROOT}/acsim/ARMMibench" "${TESTROOT}/acsim/ARMSpec" "${ARMREV}" "powersc"
    fi
    if [ "$SPARC_POWERSC" != "no" ]; then
        echo -ne "\n Running Sparc... \n"
        run_tests_acsim "sparc" "${TESTROOT}/acsim/SparcMibench" "${TESTROOT}/acsim/SparcSpec" "${SPARCREV}" "powersc"
    fi
    if [ "$MIPS_POWERSC" != "no" ]; then
        echo -ne "\n Running Mips... \n"
        run_tests_acsim "mips" "${TESTROOT}/acsim/MipsMibench" "${TESTROOT}/acsim/MipsSpec" "${MIPSREV}" "powersc"
    fi
    if [ "$POWERPC_POWERSC" != "no" ]; then
        echo -ne "\n Running PowerPC... \n"
        run_tests_acsim "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${TESTROOT}/acsim/PowerPCSpec" "${PPCREV}" "powersc"
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

####################################
### ENTRY POINT
####################################

# Initializing HTML log files
# Discover this run's number and prefix all our HTML files with it
export HTMLPREFIX=`sed -n -e '/<tr><td>[0-9]\+/{s/<tr><td>\([0-9]\+\).*/\1/;p;q}' <${HTMLINDEX}`
export LASTHTMLPREFIX=$HTMLPREFIX

export LASTARCHCREV=`grep -e "<tr><td>" < ${HTMLINDEX} | head -n 1 | cut -d\> -f 7 | cut -d\< -f 1`
export LASTEQCURRENT="true"


HTMLPREFIX=$(($HTMLPREFIX + 1))

HTMLLOG=${LOGROOT}/${HTMLPREFIX}-index.htm

initialize_html $HTMLLOG "NightlyTester ${NIGHTLYVERSION} Run #${HTMLPREFIX}"
export DATE=`LANG=en_US date '+%a %D %T'`
echo -ne "<p>Produced by NightlyTester @ ${DATE}</p>"   >> $HTMLLOG
echo -ne "<h3>Listing of GIT links used in this run.</h3>\n" >> $HTMLLOG
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
echo -ne "<tr><th>Component</th><th>Link</th></tr>\n" >> $HTMLLOG
if [ -z "$CLONELINK" ]; then
  echo -ne "<tr><td>ArchC</td><td>${WORKINGCOPY}</td></tr>\n" >> $HTMLLOG
  LASTEQCURRENT="false"
else
  echo -ne "<tr><td>ArchC</td><td>${CLONELINK}</td></tr>\n" >> $HTMLLOG
fi

if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_ARM_ACASM" != "no" -o "$RUN_ARM_ACCSIM" != "no" ]; then
  if [ -z "$ARMGITLINK" ]; then
    echo -ne "<tr><td>ARM Model</td><td>${ARMWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="false"
  else
    echo -ne "<tr><td>ARM Model</td><td>${ARMGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_MIPS_ACSIM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" ]; then
  if [ -z "$MIPSGITLINK" ]; then
    echo -ne "<tr><td>MIPS Model</td><td>${MIPSWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="false"
  else
    echo -ne "<tr><td>MIPS Model</td><td>${MIPSGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_SPARC_ACSIM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" ]; then
  if [ -z "$SPARCGITLINK" ]; then
    echo -ne "<tr><td>SPARC Model</td><td>${SPARCWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="false"
  else
   echo -ne "<tr><td>SPARC Model</td><td>${SPARCGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_POWERPC_ACASM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
   if [ -z "$POWERPCGITLINK" ]; then
    echo -ne "<tr><td>POWERPC Model</td><td>${POWERPCWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="false"
  else
    echo -ne "<tr><td>POWERPC Model</td><td>${POWERPCGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
echo -ne "</table></p>\n" >> $HTMLLOG

if [ "$SYSTEMCCOMPILE" != "yes" ]; then
  echo -ne "<p>User-supplied SystemC path: ${SYSTEMCPATH}</p>\n" >> $HTMLLOG
fi

if [ "$RUN_POWERSC" != "no" ]; then
    echo -ne "<p>User-supplied PowerSC path: ${POWERSCPATH}</p>\n" >> $HTMLLOG
fi

# GIT clone and ArchC build configuration
mkdir ${TESTROOT}
mkdir ${TESTROOT}/acsrc
mkdir ${TESTROOT}/install
cd ${TESTROOT}/acsrc

if [ -z "$CLONELINK" ]; then
  echo -ne "Copying ArchC source from a local directory...\n"
  cp -a ${WORKINGCOPY} ./ &> /dev/null
  make distclean &> /dev/null
  [ $? -ne 0 ] && {
    echo -ne "<p><b><font color=\"crimson\">ArchC source copy failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html $HTMLLOG ""
    echo -ne "Local directory copy \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  }
  ARCHCREV="N/A"
else
  echo -ne "Cloning ArchC GIT version...\n"
  git clone $CLONELINK . > /dev/null 2>&1 
  [ $? -ne 0 ] && {
    rm $TEMPFL
    echo -ne "<p><b><font color=\"crimson\">ArchC GIT clone failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html $HTMLLOG ""
    echo -ne "GIT clone \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  } 
  # Extract revision number
  # ARCHCREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
  cd ${TESTROOT}/acsrc &> /dev/null
  #ARCHCREV=$(git log | head -n1 | cut -c8-13)"..."$(git log | head -n1 | cut -c42-)
  ARCHCREV=$(git log | head -n1 | cut -c8-15)".."
  if [ ${ARCHCREV} != ${LASTARCHCREV} ]; then
        LASTEQCURRENT="false"
  fi
  cd - &> /dev/null
  #rm $TEMPFL
fi

# binutils
if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
    echo -ne "<p>Using user-supplied Binutils path: ${BINUTILSPATH}</p>\n" >> $HTMLLOG
    if [ -d $BINUTILSPATH ]; then
        echo -ne "Directory binutils found...\n"
        mkdir ${TESTROOT}/binutils
        cd ${TESTROOT}/binutils
        cp -r $BINUTILSPATH .
        BINUTILSPATH=${TESTROOT}/binutils/$(basename $BINUTILSPATH)
    elif [ -f $BINUTILSPATH ]; then
            echo -ne "Uncompressing binutils...\n"    
            mkdir ${TESTROOT}/binutils
            cd ${TESTROOT}/binutils
            tar -xjf $BINUTILSPATH
    else
        echo -ne "ACASM enabled and binutils not found.\n"
        do_abort
    fi
fi

## gdb
## Only decompress if running acsim tests (gdb is used to validate correct execution of acstone benchmark)
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  if [ "$RUN_ACSTONE" != "no" ]; then
    echo -ne "<p>Using user-supplied GDB path: ${GDBPATH}</p>\n" >> $HTMLLOG
    echo -ne "Copying GDB source...\n"
    mkdir ${TESTROOT}/gdb
    cd ${TESTROOT}/gdb
    cp -r $GDBPATH .
#    wget http://www.ic.unicamp.br/~auler/fix-gdb-6.4.patch > /dev/null 2>&1
#    patch -p1 < ./fix-gdb-6.4.patch 
    GDBPATH=${TESTROOT}/gdb/$(basename $GDBPATH)
  fi
fi

## systemc
#if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o 
#   "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o 
#   "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
#  if [ "$SYSTEMCCOMPILE" != "no" ]; then
#    echo -ne "Uncompressing SystemC...\n"
#    # only compile SystemC if we will run ACSIM/ACCSIM tests
#    mkdir ${TESTROOT}/systemc
#    cd ${TESTROOT}/systemc
#    tar -xjf ${SCRIPTROOT}/sources/systemc-2.2.0.tar.bz2
#  fi
#fi

### Configure & install ArchC
cd ${TESTROOT}/acsrc
echo -ne "Building/Installing ArchC...\n"
TEMPFL=${RANDOM}.out

# Configure script
ACSIM_STRING=""
ACASM_STRING=""
ACSTONE_STRING=""
POWERSC_STRING=""
./boot.sh > $TEMPFL 2>&1
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o  \
     "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
    ACSIM_STRING="--with-systemc=${SYSTEMCPATH}"
fi
if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
    ACASM_STRING="--with-binutils=${BINUTILSPATH}"
fi
if [ "$RUN_ACSTONE" != "no" ]; then
    ACSTONE_STRING=" --with-gdb=${GDBPATH}"
fi
if [ "$RUN_POWERSC" != "no" ]; then
    POWERSC_STRING="--with-powersc=${POWERSCPATH}"
fi
./configure --prefix=${TESTROOT}/install $ACSIM_STRING $ACASM_STRING $ACSTONE_STRING $POWERSC_STRING >> $TEMPFL 2>&1    

# Compile
make >> $TEMPFL 2>&1 &&
make install >> $TEMPFL 2>&1
RETCODE=$?
HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-archc-build-log.htm
initialize_html $HTMLBUILDLOG "ArchC rev $ARCHCREV build output"
format_html_output $TEMPFL $HTMLBUILDLOG
finalize_html $HTMLBUILDLOG ""
rm $TEMPFL
if [ $RETCODE -ne 0 ]; then
  echo -ne "<p><b><font color=\"crimson\">ArchC rev. $ARCHCREV build failed. Check <a href=\"${HTMLPREFIX}-archc-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG
  echo -ne "ArchC build \e[31mfailed\e[m.\n"
  finalize_html $HTMLLOG ""
  do_abort
else
  echo -ne "<p>ArchC rev. $ARCHCREV built successfully. Check <a href=\"${HTMLPREFIX}-archc-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
fi

### Get ArchC Models
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_ARM_ACASM" != "no" -o "$RUN_ARM_ACCSIM" != "no" ]; then
  clone_or_copy_model "arm" "${ARMGITLINK}" "${ARMWORKINGCOPY}" 
  ARMREV=${MODELREV}
fi
if [ "$RUN_SPARC_ACSIM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" ]; then
  clone_or_copy_model "sparc" "${SPARCGITLINK}" "${SPARCWORKINGCOPY}" 
  SPARCREV=${MODELREV}
fi
if [ "$RUN_MIPS_ACSIM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" ]; then
  clone_or_copy_model "mips" "${MIPSGITLINK}" "${MIPSWORKINGCOPY}" 
  MIPSREV=${MODELREV}
fi
if [ "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_POWERPC_ACASM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  clone_or_copy_model "powerpc" "${POWERPCGITLINK}" "${POWERPCWORKINGCOPY}" 
  PPCREV=${MODELREV}
fi

# If All Revision tested in last execution, is not generated a new entry in the table
if [ "$LASTEQCURRENT" != "false" ]; then
    echo -ne "All Revisions tested in last execution\n"
    rm ${LOGROOT}/${HTMLPREFIX}-* 
    do_abort
fi

mkdir ${TESTROOT}/acsim

# Golden Environment             
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o     \
     "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o \
     "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "Uncompressing correct results for Mibench...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/GoldenMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  if is_spec2006_enabled; then
      echo -ne "Uncompressing correct results for SPEC2006...\n"
    cd ${TESTROOT}/acsim
    tar -xjf ${SCRIPTROOT}/sources/GoldenSPEC2006.tar.bz2
    [ $? -ne 0 ] && do_abort
  fi
fi

# Building ARM Test Environment  
if [ "$RUN_ARM_ACSIM" != "no" ]; then   
  if [ "$COMPILE" != "no" ]; then
    echo -ne "Uncompressing Mibench from source to ARM cross compiling...\n"
    tar -xjf ${SCRIPTROOT}/sources/SourceLittleEndianMibench.tar.bz2
    [ $? -ne 0 ] && do_abort
    mv SourceLittleEndianMibench ARMMibench

    if is_spec2006_enabled; then
        echo -ne "Uncompressing SPEC2006 from source to ARM cross compiling...\n"
        #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
        cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
        [ $? -ne 0 ] && do_abort
        mv SourceSPEC2006 ARMSpec
    fi
    export TESTCOMPILER=$CROSS_ARM/`ls $CROSS_ARM | grep gcc$` 
    export TESTCOMPILERCPP=$CROSS_ARM/`ls $CROSS_ARM | grep g++$` 
    export TESTAR=$CROSS_ARM/`ls $CROSS_ARM | grep "\-ar$" | grep -v gcc` 
    export TESTRANLIB=$CROSS_ARM/`ls $CROSS_ARM | grep ranlib$ | grep -v gcc`
    export TESTFLAG=$CROSS_ARM_FLAG
  else
    echo -ne "Uncompressing Mibench precompiled for ARM...\n"
    tar -xjf ${SCRIPTROOT}/sources/ARMMibench.tar.bz2
    [ $? -ne 0 ] && do_abort
    
    #FIXME: Make precompiled for SPEC2006
    if is_spec2006_enabled; then
       echo -ne "SPEC precompiled unavailable\n"
       do_abort
    fi
  fi
fi

# Building SPARCV8 Test Environment 
if [ "$RUN_SPARC_ACSIM" != "no" ]; then
  if [ "$COMPILE" != "no" ]; then
     echo -ne "Uncompressing Mibench from source to SPARC cross compiling...\n"
     tar -xjf ${SCRIPTROOT}/sources/SourceBigEndianMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceBigEndianMibench SparcMibench
     echo -ne "Uncompressing SPEC2006 from source to SPARC cross compiling...\n"
     #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
     cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
     [ $? -ne 0 ] && do_abort
     mv SourceSPEC2006 SparcSpec
     export TESTCOMPILER=$CROSS_SPARC/`ls $CROSS_SPARC | grep gcc$` 
     export TESTCOMPILERCPP=$CROSS_SPARC/`ls $CROSS_SPARC | grep g++$` 
     export TESTAR=$CROSS_SPARC/`ls $CROSS_SPARC | grep "\-ar$" | grep -v gcc` 
     export TESTRANLIB=$CROSS_SPARC/`ls $CROSS_SPARC | grep ranlib$ | grep -v gcc`
     export TESTFLAG=$CROSS_SPARC_FLAG
  else
    echo -ne "Uncompressing Mibench precompiled for SPARC...\n"
    cd ${TESTROOT}/acsim
    tar -xjf ${SCRIPTROOT}/sources/SparcMibench.tar.bz2
    [ $? -ne 0 ] && do_abort
  fi
fi

# Building MIPS Test Environment    
if [ "$RUN_MIPS_ACSIM" != "no" ]; then
  if [ "$COMPILE" != "no" ]; then
     echo -ne "Uncompressing Mibench from source to MIPS cross compiling...\n"
     tar -xjf ${SCRIPTROOT}/sources/SourceBigEndianMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceBigEndianMibench MipsMibench
     echo -ne "Uncompressing SPEC2006 from source to MIPS cross compiling...\n"
     #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
     cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
     [ $? -ne 0 ] && do_abort
     mv SourceSPEC2006 MipsSpec
     export TESTCOMPILER=$CROSS_MIPS/`ls $CROSS_MIPS | grep gcc$` 
     export TESTCOMPILERCPP=$CROSS_MIPS/`ls $CROSS_MIPS | grep g++$` 
     export TESTAR=$CROSS_MIPS/`ls $CROSS_MIPS | grep "\-ar$" | grep -v gcc` 
     export TESTRANLIB=$CROSS_MIPS/`ls $CROSS_MIPS | grep ranlib$ | grep -v gcc`
     export TESTFLAG=$CROSS_MIPS_FLAG
 else
    cd ${TESTROOT}/acsim
    echo -ne "Uncompressing Mibench precompiled for MIPS...\n"
    tar -xjf ${SCRIPTROOT}/sources/MipsMibench.tar.bz2
    [ $? -ne 0 ] && do_abort
  fi
fi

# Building PowerPC Test Environment  
if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
  if [ "$COMPILE" != "no" ]; then
     echo -ne "Uncompressing Mibench from source to POWERPC cross compiling...\n"
     tar -xjf ${SCRIPTROOT}/sources/SourceBigEndianMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceBigEndianMibench PowerPCMibench
     echo -ne "Uncompressing SPEC2006 from source to POWERPC cross compiling...\n"
     #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
     cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
     [ $? -ne 0 ] && do_abort
     mv SourceSPEC2006 PowerPCSpec
     export TESTCOMPILER=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep gcc$` 
     export TESTCOMPILERCPP=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep g++$` 
     export TESTAR=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep "\-ar$" | grep -v gcc` 
     export TESTRANLIB=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep ranlib$ | grep -v gcc`
     export TESTFLAG=$CROSS_POWERPC_FLAG
  else 
     echo -ne "Uncompressing Mibench precompiled for PowerPC...\n"
     cd ${TESTROOT}/acsim
     tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
  fi
fi

#########################
###  Call the tests
#########################

if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
    test_acsim_simple
fi

if [ $RUN_POWERSC != "no" ]; then
    test_powersc
fi

if [ "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
    test_accsim
fi

if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
    test_acasm
fi

if [ $RUN_ACSTONE != "no" ]; then
    test_acstone
fi


#########################

finalize_nightly_tester

exit 0
