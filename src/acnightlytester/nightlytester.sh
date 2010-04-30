#!/bin/bash

# NightlyTester script for ArchC.
# Checkout ArchC source in SVN and tests current version
#
# ArchC Team

# Parameters adjustable by environment variables

# Tests switches (yes/no)
RUN_ARM_ACSIM=yes
RUN_MIPS_ACSIM=yes
RUN_SPARC_ACSIM=yes
RUN_POWERPC_ACSIM=yes
RUN_ARM_ACASM=yes
RUN_MIPS_ACASM=yes
RUN_SPARC_ACASM=yes
RUN_POWERPC_ACASM=yes
DELETEWHENDONE=yes

# Internal parameters
SCRIPTROOT=`pwd`
TESTROOT=${SCRIPTROOT}/TEMP${RANDOM}
LOGROOT=${SCRIPTROOT}/htmllogs
HTMLINDEX=${LOGROOT}/index.htm
NIGHTLYVERSION=0.1
CHECKOUTLINK=http://lampiao.lsc.ic.unicamp.br/svn/archc-prj/archc/branches/archc-newbingen-branch/
ARMSVNLINK=http://lampiao.lsc.ic.unicamp.br/svn/archc-prj/models/armv5/branches/armv5-newbingen-branch/
SPARCSVNLINK=http://lampiao.lsc.ic.unicamp.br/svn/archc-prj/models/sparcv8/trunk/
MIPSSVNLINK=http://lampiao.lsc.ic.unicamp.br/svn/archc-prj/models/mips1/trunk/
POWERPCSVNLINK=http://lampiao.lsc.ic.unicamp.br/svn/archc-prj/models/powerpc/trunk/

# Constants 
SYSTEMCPATH=${TESTROOT}/systemc/install
TLMPATH=${TESTROOT}/tlm/TLM-2005-04-08/tlm
BINUTILSPATH=${TESTROOT}/binutils/binutils-2.15
GDBPATH=${TESTROOT}/gdb/gdb-6.4
GCCPATH=${TESTROOT}/gcc/gcc-3.3

# Functions

finalize_nightly_tester() {
  TEMPFL=${random}.out
  HTMLLINE="<tr><td>${HTMLPREFIX}</td><td>${DATE}</td><td>${ARCHCREV}</td><td><a href=\"${HTMLPREFIX}-index.htm\">Here</a></td></tr>"
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

  TEMPFL=${random}.out
  svn co ${SVNLINK} ${TESTROOT}/${MODELNAME} > $TEMPFL 2>&1
  [ $? -ne 0 ] && {
    rm $TEMPFL
    echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model SVN checkout failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
    finalize_html "" $HTMLLOG  
    echo -ne "SVN checkout \e[31mfailed\e[m. Check script parameters.\n"
    do_abort
  } 
  # Extract model revision number
  MODELREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
  rm $TEMPFL

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
    echo -ne "Building ${MODELNAME} ArchC Model...\n"

    cd ${TESTROOT}/${MODELNAME}
    TEMPFL=${random}.out
    ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac -nw -abi > $TEMPFL 2>&1 && make -f Makefile.archc >> $TEMPFL 2>&1
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
  mkdir binutils
  echo -ne "Building ${MODELNAME} BINUTILS ArchC Model...\n"
  TEMPFL=${random}.out
  ${TESTROOT}/install/bin/acbingen.sh ${MODELNAME}.ac > $TEMPFL 2>&1 &&
    mkdir build-binutils &&
    cd build-binutils &&
    ${BINUTILSPATH}/configure --target=${MODELNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1 #&& won't build gdb
    #cd .. &&
    #mkdir build-gdb &&
    #cd build-gdb &&
    #${GDBPATH}/configure --target=${MODELNAME}-elf --prefix=${TESTROOT}/${MODELNAME}/binutils >> $TEMPFL 2>&1 &&
    #make >> $TEMPFL 2>&1 &&
    #make install >> $TEMPFL 2>&1
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
  TEMPFL=${random}.out
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
# This function is used to run simulation tests using Mibench and ArchC's generated simulator for a target architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is mibench root (e.g. "${TESTROOT}/acsim/ARMMibench")
# Param3 is model's svn revision number (e.g. "${ARMREV})
# e.g. run_tests_acsim "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"
run_tests_acsim() {
  MODELNAME=$1
  MODELBENCHROOT=$2
  MODELREV=$3

  # Preparing test script
  ARCH="${MODELNAME}"
  SIMULATOR="${TESTROOT}/${MODELNAME}/${MODELNAME}.x --load="
  RUNSMALL=yes
  RUNLARGE=no
  COMPILE=no
  GOLDENROOT=${TESTROOT}/acsim/GoldenMibench
  BENCHROOT=${MODELBENCHROOT}  
  export ARCH
  export SIMULATOR 
  export RUNSMALL
  export RUNLARGE
  export COMPILE
  export GOLDENROOT
  export BENCHROOT

  # Define which programs to test
  BASICMATH=yes # expensive
  BITCOUNT=yes
  QUICKSORT=yes
  SUSAN=yes
  ADPCM=yes
  CRC=yes
  FFT=yes # expensive
  GSM=yes
  DIJKSTRA=yes
  PATRICIA=yes # expensive
  RIJNDAEL=yes
  SHA=yes
  JPEG=yes
  LAME=yes # expensive
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
  
  echo -ne "<tr><td>${MODELNAME}</td><td>${MODELREV}</td><td><a href=\"${HTMLPREFIX}-${MODELNAME}.htm\">Here</a></td></tr>\n" >> $HTMLLOG
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
  LOG_FILE=l${random}.out
  FORM_FILE=f${random}.out
  HTML_LOG_FILE=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-acasm-mibench-report.htm
  export LOG_FILE
  export FORM_FILE
  export HTML_LOG_FILE
  ../../runtest.sh --verbose-log ../mibench.conf
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
echo -ne "<tr><td>ArchC</td><td>${CHECKOUTLINK}</td></tr>\n" >> $HTMLLOG
echo -ne "<tr><td>ARM Model</td><td>${ARMSVNLINK}</td></tr>\n" >> $HTMLLOG
echo -ne "<tr><td>MIPS Model</td><td>${MIPSSVNLINK}</td></tr>\n" >> $HTMLLOG
echo -ne "<tr><td>SPARC Model</td><td>${SPARCSVNLINK}</td></tr>\n" >> $HTMLLOG
echo -ne "<tr><td>PowerPC Model</td><td>${POWERPCSVNLINK}</td></tr>\n" >> $HTMLLOG
echo -ne "</table></p>\n" >> $HTMLLOG

# SVN checkout and ArchC build configuration
mkdir ${TESTROOT}
mkdir ${TESTROOT}/acsrc
mkdir ${TESTROOT}/install
cd ${TESTROOT}/acsrc

echo -ne "Checking out ArchC SVN version...\n"
TEMPFL=${random}.out
svn co ${CHECKOUTLINK} ./ > $TEMPFL 2>&1
[ $? -ne 0 ] && {
  rm $TEMPFL
  echo -ne "<p><b><font color=\"crimson\">ArchC SVN checkout failed. Check script parameters.</font></b></p>\n" >> $HTMLLOG
  finalize_html "" $HTMLLOG
  echo -ne "SVN checkout \e[31mfailed\e[m. Check script parameters.\n"
  do_abort
} 
# Extract revision number
ARCHCREV=`sed -n -e '/Checked out revision/{s/Checked out revision \+\([0-9]\+\).*/\1/;p}' <$TEMPFL`
rm $TEMPFL

###########################################
### Unpack necessary software packages
###########################################

echo -ne "Uncompressing auxiliary software packages...\n"

# binutils
mkdir ${TESTROOT}/binutils
cd ${TESTROOT}/binutils
tar -xjf ${SCRIPTROOT}/sources/binutils-2.15.tar.bz2

# gdb
mkdir ${TESTROOT}/gdb
cd ${TESTROOT}/gdb
tar -xjf ${SCRIPTROOT}/sources/gdb-6.4.tar.bz2

# gcc
mkdir ${TESTROOT}/gcc
cd ${TESTROOT}/gcc
tar -xf ${SCRIPTROOT}/sources/gcc-3.3.tar.gz

# tlm
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  # only compile SystemC if we will run ACSIM tests
  mkdir ${TESTROOT}/tlm
  cd ${TESTROOT}/tlm
  tar -xf ${SCRIPTROOT}/sources/TLM-1.0.tar.gz
fi

# systemc
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  # only compile SystemC if we will run ACSIM tests
  mkdir ${TESTROOT}/systemc
  cd ${TESTROOT}/systemc
  tar -xjf ${SCRIPTROOT}/sources/systemc-2.2.0.tar.bz2
fi

###############################################
### Configuring SystemC and building library
###############################################
mkdir install
cd install
# Only compile SystemC if we will run ACSIM tests
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  TEMPFL=${random}.out
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
    finalize_html "" $HTMLLOG  
    echo -ne "SystemC build \e[31mfailed\e[m.\n"
    do_abort
  } 
  rm $TEMPFL
  TEMPFL=${random}.out
  echo -ne "Installing SystemC...\n"
  make install > $TEMPFL 2>&1
  [ $? -ne 0 ] && {
    HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-systemc-install-log.htm
    initialize_html $HTMLBUILDLOG "SystemC install output"
    format_html_output $TEMPFL $HTMLBUILDLOG
    finalize_html $HTMLBUILDLOG ""
    rm $TEMPFL
    echo -ne "<p><b><font color=\"crimson\">SystemC install failed. Check <a href=\"${HTMLPREFIX}-systemc-install-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG
    finalize_html "" $HTMLLOG    
    echo -ne "SystemC install \e[31mfailed\e[m.\n"
    do_abort
  } 
  rm $TEMPFL
fi
########################################
### Configure & install ArchC
########################################
cd ${TESTROOT}/acsrc

echo -ne "Building/Installing ArchC...\n"
TEMPFL=${random}.out
# Configure script
./boot.sh > $TEMPFL 2>&1
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  # Configure ArchC with SystemC, because we will need acsim
  ./configure --prefix=${TESTROOT}/install --with-systemc=${SYSTEMCPATH} --with-tlm=${TLMPATH} --with-binutils=${BINUTILSPATH} --with-gdb=${GDBPATH} >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1
  RETCODE=$?
else
  # Configure ArchC without SystemC, because we won't need acsim
  ./configure --prefix=${TESTROOT}/install --with-binutils=${BINUTILSPATH} --with-gdb=${GDBPATH} >> $TEMPFL 2>&1 &&
    make >> $TEMPFL 2>&1 &&
    make install >> $TEMPFL 2>&1
  RETCODE=$?
fi
HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-archc-build-log.htm
initialize_html $HTMLBUILDLOG "ArchC rev $ARCHCREV build output"
format_html_output $TEMPFL $HTMLBUILDLOG
finalize_html $HTMLBUILDLOG ""
rm $TEMPFL
if [ $RETCODE -ne 0 ]; then
  echo -ne "<p><b><font color=\"crimson\">ArchC rev. $ARCHCREV build failed. Check <a href=\"${HTMLPREFIX}-archc-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG
  echo -ne "ArchC build \e[31mfailed\e[m.\n"
  finalize_html "" $HTMLLOG    
  do_abort
else
  echo -ne "<p>ArchC rev. $ARCHCREV built successfully. Check <a href=\"${HTMLPREFIX}-archc-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
fi

######################
### ArchC's models
######################

### Build ARM Model
build_model "armv5e" "${ARMSVNLINK}" "${RUN_ARM_ACSIM}"
ARMREV=${MODELREV}
if [ "$RUN_ARM_ACASM" != "no" ]; then
  build_binary_tools "armv5e"
  build_original_toolchain "armv5e" "arm"
fi

### Build sparcv8 Model
build_model "sparcv8" "${SPARCSVNLINK}" "${RUN_SPARC_ACSIM}"
SPARCREV=${MODELREV}
if [ "$RUN_SPARC_ACASM" != "no" ]; then
  build_binary_tools "sparcv8"
  build_original_toolchain "sparcv8" "sparc"
fi

### Build mips1 Model
build_model "mips1" "${MIPSSVNLINK}" "${RUN_MIPS_ACSIM}"
MIPSREV=${MODELREV}
if [ "$RUN_MIPS_ACASM" != "no" ]; then
  build_binary_tools "mips1"
  build_original_toolchain "mips1" "mips"
fi

### Build powerpc Model
build_model "powerpc" "${POWERPCSVNLINK}" "${RUN_POWERPC_ACSIM}"
 ### in powerpc model we change the name of the model to "powerpc1" to avoid conflicts in acbinutils validation
PPCREV=${MODELREV}
if [ "$RUN_POWERPC_ACASM" != "no" ]; then
  build_binary_tools "powerpc1"
  build_original_toolchain "powerpc1" "powerpc"
fi

### Test enviroment setup
mkdir ${TESTROOT}/acsim
cp ${SCRIPTROOT}/validation.sh ${TESTROOT}/acsim/validation.sh
chmod u+x ${TESTROOT}/acsim/validation.sh
export LOGROOT
export HTMLPREFIX

echo -ne "<h3>Listing of component versions tested in this run.</h3>\n" >> $HTMLLOG
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
echo -ne "<tr><th>Component</th><th>Version</th><th>Report</th></tr>\n" >> $HTMLLOG
echo -ne "<tr><td>ACSIM simulating Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG

######################################
### Building ARM Test Environment  ###
######################################
if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
  echo -ne "Uncompressing correct results for Mibench...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/GoldenMibench.tar.bz2
  [ $? -ne 0 ] && do_abort
fi

if [ "$RUN_ARM_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for ARM...\n"
  tar -xjf ${SCRIPTROOT}/sources/ARMMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"
fi

##########################################
### Building SPARCV8 Test Environment  ###
##########################################

if [ "$RUN_SPARC_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for SPARC...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/SparcMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim "sparcv8" "${TESTROOT}/acsim/SparcMibench" "${SPARCREV}"
fi

##########################################
### Building MIPS Test Environment     ###
##########################################

if [ "$RUN_MIPS_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for MIPS...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/MipsMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim "mips1" "${TESTROOT}/acsim/MipsMibench" "${MIPSREV}"
fi

##########################################
### Building PowerPC Test Environment  ###
##########################################

if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
  echo -ne "Uncompressing Mibench precompiled for PowerPC...\n"
  cd ${TESTROOT}/acsim
  tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
  [ $? -ne 0 ] && do_abort

  run_tests_acsim "powerpc1" "${TESTROOT}/acsim/PowerPCMibench" "${PPCREV}"
fi

#################################
### ACASM VALIDATION TESTS
#################################
echo -ne "Uncompressing ACASM validation package...\n"
cd $TESTROOT
tar -xjf ${SCRIPTROOT}/sources/acasm-validation.tar.bz2
[ $? -ne 0 ] && do_abort
chmod u+x acasm-validation/runtest.sh
echo -ne "<tr><td>ACASM assembling and linking Mibench apps</td><td></td><td></td></tr>\n" >> $HTMLLOG
# ARM
if [ "$RUN_ARM_ACASM" != "no" ]; then
  run_tests_acasm "armv5e" "arm"
fi

# MIPS
if [ "$RUN_MIPS_ACASM" != "no" ]; then
  run_tests_acasm "mips1" "mips"
fi

# SPARC
if [ "$RUN_SPARC_ACASM" != "no" ]; then
  run_tests_acasm "sparcv8" "sparc"
fi

# POWERPC
if [ "$RUN_POWERPC_ACASM" != "no" ]; then
  run_tests_acasm "powerpc1" "powerpc"
fi

################################
# Finalizing
################################
finalize_html $HTMLLOG "</table></p>"

finalize_nightly_tester

exit 0
