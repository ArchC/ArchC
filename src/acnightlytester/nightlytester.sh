#!/bin/bash

# NightlyTester script for ArchC.
# Checkout ArchC source in SVN and tests current version
#
# ArchC Team

# Parameters adjustable by environment variables

if ! [ -f "nightlytester.conf" ]; then
  echo "Configuration file not found. Must run in the same directory which contains the configuration file."
  exit 1
fi

. nightlytester.conf

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

TLMPATH=${TESTROOT}/tlm/TLM-2005-04-08/tlm
BINUTILSPATH=${TESTROOT}/binutils/binutils-2.15
GDBPATH=${TESTROOT}/gdb/gdb-6.4
GCCPATH=${TESTROOT}/gcc/gcc-3.3

# Functions

finalize_nightly_tester() {
  TEMPFL=${RANDOM}.out
  HTMLLINE="<tr><td>${HTMLPREFIX}</td><td>${DATE}</td><td>${ARCHCREV}</td><td><a href=\"${HTMLPREFIX}-index.htm\">Here</a></td><td>${REVMESSAGE}</td><td>${HOSTNAME}</td></tr>"
  sed -e "/<tr><td>${LASTHTMLPREFIX}/i${HTMLLINE}" $HTMLINDEX > $TEMPFL
  mv ${TEMPFL} $HTMLINDEX

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

#
# This function is used to build a model using ArchC's simulator generator ACSIM 
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is SVN link (e.g. "$ARMSVNLINK")
# Param3 says if we really should build using ACSIM, or just do the svn checkout (e.g. "$RUN_ARM_ACSIM")
# e.g. ARMREV=build_model "armv5e" "$ARMSVNLINK" "$RUN_ARM_ACSIM"
build_model() {
  MODELNAME=$1
  SVNLINK=$2
  USEACSIM=$3
  mkdir ${TESTROOT}/${MODELNAME}
  echo -ne "Checking out ${MODELNAME} ArchC Model SVN...\n"

  TEMPFL=${RANDOM}.out
  svn co ${SVNLINK} ${TESTROOT}/${MODELNAME} > $TEMPFL 2>&1
  [ $? -ne 0 ] && {
    rm $TEMPFL
    echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model SVN checkout failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html $HTMLLOG ""
    echo -ne "SVN checkout \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  } 
  # Extract model revision number
  MODELREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
  rm $TEMPFL

  ### WORKAROUND: In sparcv8 model, we must rename it to "sparcv8_1" to enable gdb compilation (in gdb tree there is already an architecture named sparcv8)
  if [ "$MODELNAME" = "sparcv8" ]; then
    MODELNAME="sparcv8_1" 
    cd ${TESTROOT}
    mv sparcv8 sparcv8_1
    cd sparcv8_1
    for MODELFILE in `find sparcv8*`
    do
	NEWFILENAME=`sed -e 's/sparcv8/sparcv8_1/' <<< "$MODELFILE"`
	sed -e 's/sparcv8/sparcv8_1/' $MODELFILE > $NEWFILENAME
    done
  fi

  ### WORKAROUND: In powerpc model, we must rename it to "powerpc1" to enable acbinutils testing (in binutils tree there is already an architecture named powerpc)
  if [ "$MODELNAME" = "powerpc" ]; then
    MODELNAME="powerpc1" 
    cd ${TESTROOT}
    mv powerpc powerpc1
    cd powerpc1
    for MODELFILE in `find powerpc*`
    do
	NEWFILENAME=`sed -e 's/powerpc/powerpc1/' <<< "$MODELFILE"`
	sed -e 's/powerpc/powerpc1/' $MODELFILE > $NEWFILENAME
    done
  fi

  if [ "$USEACSIM" != "no" ]; then    
    if [ "$RUN_ACSTONE" != "no" ]; then
      echo -ne "Building ${MODELNAME} ArchC Model with gdb support for acstone...\n"
      cd ${TESTROOT}/${MODELNAME}
      mkdir -p acstone
      cd acstone
      find ../ -maxdepth 1 -mindepth 1 -type f -exec cp '{}' . \; 
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
    echo -ne "Building ${MODELNAME} ArchC Model...\n"
    cd ${TESTROOT}/${MODELNAME}
    TEMPFL=${RANDOM}.out
    ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac ${ACSIM_PARAMS} > $TEMPFL 2>&1 && make -f Makefile.archc >> $TEMPFL 2>&1
    RETCODE=$?
    HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-build-log.htm
    initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV build output"
    format_html_output $TEMPFL $HTMLBUILDLOG
    finalize_html $HTMLBUILDLOG ""
    rm $TEMPFL
    if [ $RETCODE -ne 0 ]; then
      echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV build failed. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
      finalize_html $HTMLLOG ""
      echo -ne "ACSIM \e[31mfailed\e[m to build $MODELNAME model.\n"
      do_abort
    else
      echo -ne "<p>${MODELNAME} Model rev. $MODELREV built successfully. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
    fi
  fi
}

#
# This function is used to build binutils tools for a given model, testing ACASM, ArchC's binary tools synthetizer.
#
# Param1 is modelname (e.g. "armv5e")
#
# Requires: Model already fetched from SVN repository and stored in ${TESTROOT}/${MODELNAME}
# Results: Binary tools for architecture $MODELNAME installed in ${TESTROOT}/${MODELNAME}/binutils
#
# Use example: build_binary_tools "armv5e"
build_binary_tools() {
  MODELNAME=$1
  cd ${TESTROOT}/${MODELNAME}
  mkdir -p binutils
  echo -ne "Building ${MODELNAME} BINUTILS ArchC Model...\n"
  TEMPFL=${RANDOM}.out
  ${TESTROOT}/install/bin/acbingen.sh -f ${MODELNAME}.ac > $TEMPFL 2>&1 &&
    mkdir build-binutils &&
    cd build-binutils && # D_FORTIFY used below is used to prevent a bug present in binutils 2.15 and 2.16
    CFLAGS="-g -O2 -D_FORTIFY_SOURCE=1" ${BINUTILSPATH}/configure --target=${MODELNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
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

#
# This function is used to build gdb for a given model, testing acstone, which uses gdb for validation.
#
# Param1 is modelname (e.g. "armv5e")
#
# Requires: Model already fetched from SVN repository and stored in ${TESTROOT}/${MODELNAME}
# Results: gdb for architecture $MODELNAME installed in ${TESTROOT}/${MODELNAME}/binutils
#
# Use example: build_binary_tools "armv5e"
build_gdb() {
  MODELNAME=$1
  cd ${TESTROOT}/${MODELNAME}
  mkdir -p binutils
  echo -ne "Building GDB for ${MODELNAME}...\n"
  TEMPFL=${RANDOM}.out
  ${TESTROOT}/install/bin/acbingen.sh -f ${MODELNAME}.ac > $TEMPFL 2>&1 &&    
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


#
# This function is used to build binutils *ORIGINAL* tools for a given architecture, so we can compare and expect the results from these tools to be correct and
# validate ArchC's generated ones.
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is architecture name used in packages (e.g. "arm")
#
# Requires: Folder ${TESTROOT}/${MODELNAME} must exist.
# Results: Complete toolchain for architecture installed in ${TESTROOT}/${MODELNAME}/binutils-orig
#
# Use example: build_original_toolchain "armv5e" "arm"
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
    ${BINUTILSPATH}/configure --target=${ARCHNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils-orig >> $TEMPFL 2>&1 &&
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

#
# This functions is used to run simulations tests using ACSTONE and ArchC's generated simulator for a target architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is model's svn revision number (e.g. "${ARMREV})
# e.g. run_tests_acsim_acstone "armv5e" "${ARMREV}"
run_tests_acsim_acstone() {
  MODELNAME=$1
  MODELREV=$2

  ### WORKAROUND: In sparcv8 model, we must rename it to "sparcv8_1", see "build_model()" for more info  
  if [ "$MODELNAME" = "sparcv8" ]; then
    MODELNAME="sparcv8_1"     
  fi
  ### WORKAROUND: In powerpc model, we must rename it to "powerpc1", see "build_model()" for more info
  if [ "$MODELNAME" = "powerpc" ]; then
    MODELNAME="powerpc1" 
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

#
# This function is used to run simulation tests using Mibench and ArchC's generated simulator for a target architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is mibench root (e.g. "${TESTROOT}/acsim/ARMMibench")
# Param3 is model's svn revision number (e.g. "${ARMREV})
# e.g. run_tests_acsim_mibench "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"
run_tests_acsim_mibench() {
  MODELNAME=$1
  MODELBENCHROOT=$2
  MODELREV=$3

  # Preparing test script
  ARCH="${MODELNAME}"
  SIMULATOR="${TESTROOT}/${MODELNAME}/${MODELNAME}.x --load="
  GOLDENROOT=${TESTROOT}/acsim/GoldenMibench
  BENCHROOT=${MODELBENCHROOT}  
  STATSROOT=${BENCHROOT}/stats
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
  ./validation.sh
  
  echo -ne "<tr><td>${MODELNAME} (mibench)</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}.htm\">Here</a></td></tr>\n" >> $HTMLLOG
}

#
# This function is used to run simulation tests using Mibench and ArchC's generated *COMPILED* simulator for a target architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is mibench root (e.g. "${TESTROOT}/acsim/ARMMibench")
# Param3 is model's svn revision number (e.g. "${ARMREV})
# Param4 is accsim flags (e.g. "-ai")
# e.g. run_tests_accsim_mibench "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}" "-ai"
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


#
# This function runs ACASM validation script, assembling and linking a series of .s files using both the generated assembler and a correct reference
# assembler (binutils for a given architecture). The results are compared between these two assemblers and validation is successful if they have no
# difference.
#
# Requires: build_original_toolchain() and build_binary_tools() already called for the given architecture
# Produces: ACASM validation for a given architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is reference architecture name in binutils (e.g. "arm")
#
# e.g. run_tests_acasm "armv5e" "arm"
run_tests_acasm() {
  MODELNAME=$1
  ARCHNAME=$2
  cd ${TESTROOT}/acasm-validation/${MODELNAME}/runtest
  export BENCH_ROOT="${TESTROOT}/acasm-validation/${MODELNAME}/benchmark/Mibench"
  export ACBIN_PATH="${TESTROOT}/${MODELNAME}/binutils/${MODELNAME}-elf/bin"
  export BINUTILS_PATH="${TESTROOT}/${MODELNAME}/binutils-orig/${ARCHNAME}-elf/bin"
  LOG_FILE=l${RANDOM}.out
  FORM_FILE=f${RANDOM}.out
  HTML_LOG_FILE=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-report.htm
  export LOG_FILE
  export FORM_FILE
  export HTML_LOG_FILE
  ../../runtest.sh --verbose-log ../mibench.conf > /dev/null 2>&1
  HTMLACASMLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-log.htm
  HTMLACASMFORM=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-form.htm
  format_html_output $LOG_FILE $HTMLACASMLOG
  format_html_output $FORM_FILE $HTMLACASMFORM
  echo -ne "<tr><td>${MODELNAME}</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-report.htm\">Report</a>, <a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-log.htm\">Log</a>, <a href=\"${HTMLPREFIX}-${MODELNAME}-acasm-mibench-form.htm\">Form</a></td></tr>\n" >> $HTMLLOG
}

####################################
### ENTRY POINT
####################################

# Initializing HTML log files
# Discover this run's number and prefix all our HTML files with it
HTMLPREFIX=`sed -n -e '/<tr><td>[0-9]\+/{s/<tr><td>\([0-9]\+\).*/\1/;p;q}' <${HTMLINDEX}`
LASTHTMLPREFIX=$HTMLPREFIX
HTMLPREFIX=$(($HTMLPREFIX + 1))

HTMLLOG=${LOGROOT}/${HTMLPREFIX}-index.htm

initialize_html $HTMLLOG "NightlyTester ${NIGHTLYVERSION} Run #${HTMLPREFIX}"
DATE=`date '+%a %D %r'`
echo -ne "<p>Produced by NightlyTester @ ${DATE}</p>"   >> $HTMLLOG
echo -ne "<h3>Listing of SVN links used in this run.</h3>\n" >> $HTMLLOG
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
echo -ne "<tr><th>Component</th><th>Link</th></tr>\n" >> $HTMLLOG
if [ -z "$CHECKOUTLINK" ]; then
  echo -ne "<tr><td>ArchC</td><td>${WORKINGCOPY} (private working copy)</td></tr>\n" >> $HTMLLOG
else
  echo -ne "<tr><td>ArchC</td><td>${CHECKOUTLINK}</td></tr>\n" >> $HTMLLOG
fi
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_ARM_ACASM" != "no" -o "$RUN_ARM_ACCSIM" != "no" ]; then
  echo -ne "<tr><td>ARM Model</td><td>${ARMSVNLINK}</td></tr>\n" >> $HTMLLOG
fi
if [ "$RUN_MIPS_ACSIM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" ]; then
  echo -ne "<tr><td>MIPS Model</td><td>${MIPSSVNLINK}</td></tr>\n" >> $HTMLLOG
fi
if [ "$RUN_SPARC_ACSIM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" ]; then
  echo -ne "<tr><td>SPARC Model</td><td>${SPARCSVNLINK}</td></tr>\n" >> $HTMLLOG
fi
if [ "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_POWERPC_ACASM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "<tr><td>PowerPC Model</td><td>${POWERPCSVNLINK}</td></tr>\n" >> $HTMLLOG
fi
echo -ne "</table></p>\n" >> $HTMLLOG
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  echo -ne "<p>Parameters used to build acsim models: ${ACSIM_PARAMS}</p>\n" >> $HTMLLOG
fi
if [ "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "<p>Parameters used to build accsim models: ${ACCSIM_PARAMS}</p>\n" >> $HTMLLOG
fi
if [ "$SYSTEMCCOMPILE" != "yes" ]; then
  echo -ne "<p>Using user-supplied SystemC path: ${SYSTEMCPATH}</p>\n" >> $HTMLLOG
fi

# SVN checkout and ArchC build configuration
mkdir ${TESTROOT}
mkdir ${TESTROOT}/acsrc
mkdir ${TESTROOT}/install
cd ${TESTROOT}/acsrc

if [ -z "$CHECKOUTLINK" ]; then
  echo -ne "Copying ArchC source from a local directory...\n"
  cp -a ${WORKINGCOPY} ./ &> /dev/null
  [ $? -ne 0 ] && {
    echo -ne "<p><b><font color=\"crimson\">ArchC source copy failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html $HTMLLOG ""
    echo -ne "Local directory copy \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  }
  ARCHCREV="N/A"
else
  echo -ne "Checking out ArchC SVN version...\n"
  TEMPFL=${RANDOM}.out
  svn co ${CHECKOUTLINK} ./ > $TEMPFL 2>&1
  [ $? -ne 0 ] && {
    rm $TEMPFL
    echo -ne "<p><b><font color=\"crimson\">ArchC SVN checkout failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html $HTMLLOG ""
    echo -ne "SVN checkout \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  } 
  # Extract revision number
  ARCHCREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
  rm $TEMPFL
fi



###########################################
### Unpack necessary software packages
###########################################
echo -ne "\n**********************************************\n"
echo -ne "* Uncompressing auxiliary software packages **\n"
echo -ne "**********************************************\n"

# binutils
if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
  echo -ne "Uncompressing binutils...\n"
  mkdir ${TESTROOT}/binutils
  cd ${TESTROOT}/binutils
  tar -xjf ${SCRIPTROOT}/sources/binutils-2.15.tar.bz2
fi

# gdb
# Only decompress if running acsim tests (gdb is used to validate correct execution of acstone benchmark)
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  if [ "$RUN_ACSTONE" != "no" ]; then
    echo -ne "Uncompressing gdb...\n"
    mkdir ${TESTROOT}/gdb
    cd ${TESTROOT}/gdb
    tar -xjf ${SCRIPTROOT}/sources/gdb-6.4.tar.bz2
  fi
fi

# gcc
#echo -ne "Uncompressing gcc...\n"
#mkdir ${TESTROOT}/gcc
#cd ${TESTROOT}/gcc
#tar -xf ${SCRIPTROOT}/sources/gcc-3.3.tar.gz

# glibc
#echo -ne "Uncompressing glibc...\n"
#mkdir ${TESTROOT}/glibc
#cd ${TESTROOT}/glibc
#tar -xjf ${SCRIPTROOT}/sources/glibc-2.3.2.tar.bz2
#cd ${TESTROOT}/glibc/glibc-2.3.2
#tar -xjf ${SCRIPTROOT}/sources/glibc-linuxthreads-2.3.2.tar.bz2

# tlm
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "Uncompressing tlm...\n"
  # only compile SystemC if we will run ACSIM/ACCSIM tests
  mkdir ${TESTROOT}/tlm
  cd ${TESTROOT}/tlm
  tar -xf ${SCRIPTROOT}/sources/TLM-1.0.tar.gz
fi

# systemc
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  if [ "$SYSTEMCCOMPILE" != "no" ]; then
    echo -ne "Uncompressing SystemC...\n"
    # only compile SystemC if we will run ACSIM/ACCSIM tests
    mkdir ${TESTROOT}/systemc
    cd ${TESTROOT}/systemc
    tar -xjf ${SCRIPTROOT}/sources/systemc-2.2.0.tar.bz2
  fi
fi


echo -ne "\n**********************************************\n"
echo -ne "* Building software packages                **\n"
echo -ne "**********************************************\n"

###############################################
### Configuring SystemC and building library
###############################################
mkdir install
cd install
# Only compile SystemC if we will run ACSIM/ACCSIM tests and the user did not supply a System
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  if [ "$SYSTEMCCOMPILE" != "no" ]; then
    TEMPFL=${RANDOM}.out
    ../systemc-2.2.0/configure --prefix=${TESTROOT}/systemc/install > $TEMPFL 2>&1
    echo -ne "Building SystemC...\n"
    make >> $TEMPFL 2>&1
    [ $? -ne 0 ] && {
      HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-systemc-build-log.htm
      initialize_html $HTMLBUILDLOG "SystemC build output"
      format_html_output $TEMPFL $HTMLBUILDLOG
      finalize_html $HTMLBUILDLOG ""
      rm $TEMPFL
      echo -ne "<p><b><font color=\"crimson\">SystemC build failed. Check <a href=\"/${HTMLPREFIX}-systemc-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG
      finalize_html $HTMLLOG ""
      echo -ne "SystemC build \e[31mfailed\e[m.\n"
      do_abort
    } 
    rm $TEMPFL
    TEMPFL=${RANDOM}.out
    echo -ne "Installing SystemC...\n"
    make install > $TEMPFL 2>&1
    [ $? -ne 0 ] && {
      HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-systemc-install-log.htm
      initialize_html $HTMLBUILDLOG "SystemC install output"
      format_html_output $TEMPFL $HTMLBUILDLOG
      finalize_html $HTMLBUILDLOG ""
      rm $TEMPFL
      echo -ne "<p><b><font color=\"crimson\">SystemC install failed. Check <a href=\"${HTMLPREFIX}-systemc-install-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG
      finalize_html $HTMLLOG ""
      echo -ne "SystemC install \e[31mfailed\e[m.\n"
      do_abort
    } 
    rm $TEMPFL
  fi
fi
########################################
### Configure & install ArchC
########################################
cd ${TESTROOT}/acsrc

echo -ne "Building/Installing ArchC...\n"
TEMPFL=${RANDOM}.out
# Configure script
./boot.sh > $TEMPFL 2>&1
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
    if [ "$RUN_ACSTONE" != "no" ]; then
      # Configure ArchC fully (testing acsim/accsim and acasm) with gdb support (used by acstone)
      ./configure --prefix=${TESTROOT}/install --with-systemc=${SYSTEMCPATH} --with-tlm=${TLMPATH} --with-binutils=${BINUTILSPATH} --with-gdb=${GDBPATH} >> $TEMPFL 2>&1    
    else
      # Configure ArchC without gdb support (acsim and acasm supported)
      ./configure --prefix=${TESTROOT}/install --with-systemc=${SYSTEMCPATH} --with-tlm=${TLMPATH} --with-binutils=${BINUTILSPATH} >> $TEMPFL 2>&1    
    fi
  else
    #Configure ArchC without acasm support
    ./configure --prefix=${TESTROOT}/install --with-systemc=${SYSTEMCPATH} --with-tlm=${TLMPATH} --with-gdb=${GDBPATH} >> $TEMPFL 2>&1
  fi
else
  # Configure ArchC without SystemC, because we won't need acsim/accsim
  ./configure --prefix=${TESTROOT}/install --with-binutils=${BINUTILSPATH} >> $TEMPFL 2>&1    
fi
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

######################
### ArchC's models
######################

### Build ARM Model
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_ARM_ACASM" != "no" -o "$RUN_ARM_ACCSIM" != "no" ]; then
  build_model "armv5e" "${ARMSVNLINK}" "${RUN_ARM_ACSIM}"
  ARMREV=${MODELREV}
  if [ "$RUN_ARM_ACASM" != "no" ]; then
    build_binary_tools "armv5e"
    build_original_toolchain "armv5e" "arm"
  fi
  if [ "$RUN_ARM_ACSIM" != "no" -a "$RUN_ACSTONE" != "no" ]; then
    build_gdb "armv5e"
  fi
fi

### Build sparcv8 Model
if [ "$RUN_SPARC_ACSIM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" ]; then
  build_model "sparcv8" "${SPARCSVNLINK}" "${RUN_SPARC_ACSIM}"
  ### in sparcv8 model we change the name of the model to "sparcv8_1" to avoid conflicts with gdb preexistent target
  SPARCREV=${MODELREV}
  if [ "$RUN_SPARC_ACASM" != "no" ]; then
    build_binary_tools "sparcv8_1"
    build_original_toolchain "sparcv8_1" "sparc"
  fi
  if [ "$RUN_SPARC_ACSIM" != "no" -a "$RUN_ACSTONE" != "no" ]; then
    build_gdb "sparcv8_1"
  fi
fi

### Build mips1 Model
if [ "$RUN_MIPS_ACSIM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" ]; then
  build_model "mips1" "${MIPSSVNLINK}" "${RUN_MIPS_ACSIM}"
  MIPSREV=${MODELREV}
  if [ "$RUN_MIPS_ACASM" != "no" ]; then
    build_binary_tools "mips1"
    build_original_toolchain "mips1" "mips"
  fi
  if [ "$RUN_MIPS_ACSIM" != "no" -a "$RUN_ACSTONE" != "no" ]; then
    build_gdb "mips1"
  fi
fi

### Build powerpc Model
if [ "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_POWERPC_ACASM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  build_model "powerpc" "${POWERPCSVNLINK}" "${RUN_POWERPC_ACSIM}"
  ### in powerpc model we change the name of the model to "powerpc1" to avoid conflicts in acbinutils validation
  PPCREV=${MODELREV}
  if [ "$RUN_POWERPC_ACASM" != "no" ]; then
    build_binary_tools "powerpc1"
    build_original_toolchain "powerpc1" "powerpc"
  fi
  if [ "$RUN_POWERPC_ACSIM" != "no" -a "$RUN_ACSTONE" != "no" ]; then
    build_gdb "powerpc1"
  fi
fi

echo -ne "\n**********************************************\n"
echo -ne "* Testing                                   **\n"
echo -ne "**********************************************\n"


### Test enviroment setup
if [ "$RUN_ACSTONE" != "no" ]; then
  # Extracts acstone binaries (arm, mips, powerpc and sparc) as well as the helper scripts
  cd ${TESTROOT}
  tar -xjf ${SCRIPTROOT}/sources/AllArchs-acstone.tar.bz2
  [ $? -ne 0 ] && do_abort
  cp ${SCRIPTROOT}/acstone_run_all.sh ${TESTROOT}/acstone &&
    cp ${SCRIPTROOT}/acstone_run_teste.sh ${TESTROOT}/acstone &&
    cp ${SCRIPTROOT}/collect_stats.py ${TESTROOT}/acstone
  [ $? -ne 0 ] && do_abort	
  chmod u+x ${TESTROOT}/acstone/*.sh
  [ $? -ne 0 ] && do_abort	
fi
mkdir ${TESTROOT}/acsim
cp ${SCRIPTROOT}/validation.sh ${TESTROOT}/acsim/validation.sh
cp ${SCRIPTROOT}/validation-accsim.sh ${TESTROOT}/acsim/validation-accsim.sh
chmod u+x ${TESTROOT}/acsim/validation.sh
export LOGROOT
export HTMLPREFIX

echo -ne "<h3>Listing of component versions tested in this run.</h3>\n" >> $HTMLLOG
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  if [ "$RUN_ACSTONE" != "no" ]; then
    echo -ne "<tr><td>ACSIM simulating Acstone and Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG
  else
    echo -ne "<tr><td>ACSIM simulating Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG
  fi
fi

######################################
### ACSTONE Benchmark Testing      ###
######################################
if [ "$RUN_ACSTONE" != "no" ]; then  
  [ "$RUN_ARM_ACSIM" != "no" ] &&
    run_tests_acsim_acstone "armv5e" "${ARMREV}"
  [ "$RUN_SPARC_ACSIM" != "no" ] &&
    run_tests_acsim_acstone "sparcv8" "${SPARCREV}"
  [ "$RUN_MIPS_ACSIM" != "no" ] &&
    run_tests_acsim_acstone "mips1" "${MIPSREV}"
  [ "$RUN_POWERPC_ACSIM" != "no" ] &&
    run_tests_acsim_acstone "powerpc" "${PPCREV}"    
fi

######################################
### Building ARM Test Environment  ###
######################################
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "Uncompressing correct results for Mibench...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/GoldenMibench.tar.bz2
  [ $? -ne 0 ] && do_abort
fi

if [ "$RUN_ARM_ACSIM" != "no" ]; then  
  echo -ne "Uncompressing Mibench precompiled for ARM...\n"
  tar -xjf ${SCRIPTROOT}/sources/ARMMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim_mibench "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"
fi

##########################################
### Building SPARCV8 Test Environment  ###
##########################################

if [ "$RUN_SPARC_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for SPARC...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/SparcMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim_mibench "sparcv8_1" "${TESTROOT}/acsim/SparcMibench" "${SPARCREV}"
fi

##########################################
### Building MIPS Test Environment     ###
##########################################

if [ "$RUN_MIPS_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for MIPS...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/MipsMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim_mibench "mips1" "${TESTROOT}/acsim/MipsMibench" "${MIPSREV}"
fi

##########################################
### Building PowerPC Test Environment  ###
##########################################

if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for PowerPC...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim_mibench "powerpc1" "${TESTROOT}/acsim/PowerPCMibench" "${PPCREV}"
fi

#################################
### ACCSIM VALIDATION TESTS
#################################

if [ "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
  echo -ne "<tr><td>ACCSIM simulating Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG
fi

#arm
if [ "$RUN_ARM_ACCSIM" != "no" ]; then  
  cd ${TESTROOT}/acsim
  if [ "$RUN_ARM_ACSIM" != "no" ]; then
    rm -rf ARMMibench
  fi
  echo -ne "Uncompressing Mibench precompiled for ARM...\n"
  tar -xjf ${SCRIPTROOT}/sources/ARMMibench.tar.bz2
  [ $? -ne 0 ] && do_abort
  run_tests_accsim_mibench "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}" "-ai"
fi

#sparc
if [ "$RUN_SPARC_ACCSIM" != "no" ]; then  
  cd ${TESTROOT}/acsim
  if [ "$RUN_SPARC_ACSIM" != "no" ]; then
    rm -rf SparcMibench
  fi
  echo -ne "Uncompressing Mibench precompiled for SPARC...\n"
  tar -xjf ${SCRIPTROOT}/sources/SparcMibench.tar.bz2
  [ $? -ne 0 ] && do_abort  
  run_tests_accsim_mibench "sparcv8_1" "${TESTROOT}/acsim/SparcMibench" "${SPARCREV}" "-opt 2"
fi

#mips
if [ "$RUN_MIPS_ACCSIM" != "no" ]; then  
  cd ${TESTROOT}/acsim
  if [ "$RUN_MIPS_ACSIM" != "no" ]; then
    rm -rf MipsMibench
  fi
  echo -ne "Uncompressing Mibench precompiled for MIPS...\n"
  tar -xjf ${SCRIPTROOT}/sources/MipsMibench.tar.bz2
  [ $? -ne 0 ] && do_abort  
  run_tests_accsim_mibench "mips1" "${TESTROOT}/acsim/MipsMibench" "${MIPSREV}" "-opt 2"
fi

#powerpc
if [ "$RUN_POWERPC_ACCSIM" != "no" ]; then  
  cd ${TESTROOT}/acsim
  if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
    rm -rf PowerPCMibench
  fi
  echo -ne "Uncompressing Mibench precompiled for POWERPC...\n"
  tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
  [ $? -ne 0 ] && do_abort
  run_tests_accsim_mibench "powerpc1" "${TESTROOT}/acsim/PowerPCMibench" "${PPCREV}" "-opt 2"
fi



#################################
### ACASM VALIDATION TESTS
#################################
if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
  echo -ne "Uncompressing ACASM validation package...\n"
  cd $TESTROOT
  tar -xjf ${SCRIPTROOT}/sources/acasm-validation.tar.bz2
  [ $? -ne 0 ] && do_abort
  chmod u+x acasm-validation/runtest.sh
  echo -ne "<tr><td>ACASM assembling and linking Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG
fi

# ARM
if [ "$RUN_ARM_ACASM" != "no" ]; then
  echo -ne "Validating binary tools generated for arm ArchC model...\n"
  run_tests_acasm "armv5e" "arm"
fi

# MIPS
if [ "$RUN_MIPS_ACASM" != "no" ]; then
  echo -ne "Validating binary tools generated for mips ArchC model...\n"
  run_tests_acasm "mips1" "mips"
fi

# SPARC
if [ "$RUN_SPARC_ACASM" != "no" ]; then
  echo -ne "Validating binary tools generated for sparc ArchC model...\n"
  run_tests_acasm "sparcv8_1" "sparc"
fi

# POWERPC
if [ "$RUN_POWERPC_ACASM" != "no" ]; then
  echo -ne "Validating binary tools generated for powerpc ArchC model...\n"
  run_tests_acasm "powerpc1" "powerpc"
fi

################################
# Finalizing
################################
finalize_html $HTMLLOG "</table></p>"

finalize_nightly_tester

exit 0
