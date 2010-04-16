#!/bin/bash

# NightlyTester script for ArchC.
# Checkout ArchC source in SVN and tests current version
#
# ArchC Team

# Parameters adjustable by environment variables

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

# Functions

do_abort() {
  echo -ne "Aborting...\n\n"
  rm -rf $TESTROOT
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
# e.g. ARMREV=build_model "armv5e" "$ARMSVNLINK"
build_model() {
  MODELNAME=$1
  SVNLINK=$2
  mkdir ${TESTROOT}/${MODELNAME}
  echo -ne "Checking out ${MODELNAME} ArchC Model SVN...\n"

  TEMPFL=${random}.out
  svn co ${SVNLINK} ${TESTROOT}/${MODELNAME} 2>&1 > $TEMPFL
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

  echo -ne "Building ${MODELNAME} ArchC Model...\n"

  cd ${TESTROOT}/${MODELNAME}
  TEMPFL=${random}.out
  ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac -nw -abi 2>&1 > $TEMPFL && make -f Makefile.archc 2>&1 >> $TEMPFL
  RETCODE=$?
  HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-build-log.htm
  initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV build output"
  format_html_output $TEMPFL $HTMLBUILDLOG
  finalize_html $HTMLBUILDLOG ""
  rm $TEMPFL
  if [ $RETCODE -ne 0 ]; then
    echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Model rev. $MODELREV build failed. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-log.htm\">log</a>.</font></b></p>\n" >> $HTMLLOG  
    finalize_html "" $HTMLLOG    
    echo -ne "ACSIM \e[31mfailed\e[m to build $MODELNAME model.\n"
    do_abort
  else
    echo -ne "<p>${MODELNAME} Model rev. $MODELREV built successfully. Check <a href=\"${HTMLPREFIX}-${MODELNAME}-build-log.htm\">compilation log</a>.</p>\n" >> $HTMLLOG
  fi
}

#
# This function is used to run simulation tests using Mibench and ArchC's generated simulator for a target architecture
#
# Param1 is modelname (e.g. "armv5e")
# Param2 is mibench root (e.g. "${TESTROOT}/acsim/ARMMibench")
# Param3 is model's svn revision number (e.g. "${ARMREV})
# e.g. run_tests "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"
run_tests() {
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
svn co ${CHECKOUTLINK} ./ 2>&1 > $TEMPFL
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

# tlm
mkdir ${TESTROOT}/tlm
cd ${TESTROOT}/tlm
tar -xf ${SCRIPTROOT}/sources/TLM-1.0.tar.gz

# systemc
mkdir ${TESTROOT}/systemc
cd ${TESTROOT}/systemc
tar -xjf ${SCRIPTROOT}/sources/systemc-2.2.0.tar.bz2

###############################################
### Configuring SystemC and building library
###############################################
mkdir install
cd install
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

########################################
### Configure & install ArchC
########################################
cd ${TESTROOT}/acsrc

echo -ne "Building/Installing ArchC...\n"
TEMPFL=${random}.out
# Configure script
./boot.sh > $TEMPFL 2>&1
./configure --prefix=${TESTROOT}/install --with-systemc=${SYSTEMCPATH} --with-tlm=${TLMPATH} --with-binutils=${BINUTILSPATH} >> $TEMPFL 2>&1 && make >> $TEMPFL 2>&1 && make install >> $TEMPFL 2>&1
RETCODE=$?
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
build_model "armv5e" "${ARMSVNLINK}"
ARMREV=${MODELREV}

### Build sparcv8 Model
build_model "sparcv8" "${SPARCSVNLINK}"
SPARCREV=${MODELREV}

### Build mips1 Model
build_model "mips1" "${MIPSSVNLINK}"
MIPSREV=${MODELREV}

### Build powerpc Model
build_model "powerpc" "${POWERPCSVNLINK}"
PPCREV=${MODELREV}

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
echo -ne "Uncompressing correct results for Mibench...\n"
cd ${TESTROOT}/acsim
tar -xjf ${SCRIPTROOT}/sources/GoldenMibench.tar.bz2
[ $? -ne 0 ] && do_abort

echo -ne "Uncompressing Mibench precompiled for ARM...\n"
tar -xjf ${SCRIPTROOT}/sources/ARMMibench.tar.bz2
[ $? -ne 0 ] && do_abort

run_tests "armv5e" "${TESTROOT}/acsim/ARMMibench" "${ARMREV}"

##########################################
### Building SPARCV8 Test Environment  ###
##########################################

echo -ne "Uncompressing Mibench precompiled for SPARC...\n"
cd ${TESTROOT}/acsim
tar -xjf ${SCRIPTROOT}/sources/SparcMibench.tar.bz2
[ $? -ne 0 ] && do_abort

run_tests "sparcv8" "${TESTROOT}/acsim/SparcMibench" "${SPARCREV}"

##########################################
### Building MIPS Test Environment     ###
##########################################

echo -ne "Uncompressing Mibench precompiled for MIPS...\n"
cd ${TESTROOT}/acsim
tar -xjf ${SCRIPTROOT}/sources/MipsMibench.tar.bz2
[ $? -ne 0 ] && do_abort

run_tests "mips1" "${TESTROOT}/acsim/MipsMibench" "${MIPSREV}"

##########################################
### Building PowerPC Test Environment  ###
##########################################

echo -ne "Uncompressing Mibench precompiled for PowerPC...\n"
cd ${TESTROOT}/acsim
tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
[ $? -ne 0 ] && do_abort

run_tests "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${PPCREV}"

################################
# Finalizing
################################

finalize_html $HTMLLOG "</table></p>"

TEMPFL=${random}.out
HTMLLINE="<tr><td>${HTMLPREFIX}</td><td>${DATE}</td><td>${ARCHCREV}</td><td><a href=\"${HTMLPREFIX}-index.htm\">Here</a></td></tr>"
sed -e "/<tr><td>${LASTHTMLPREFIX}/i${HTMLLINE}" $HTMLINDEX > $TEMPFL
mv ${TEMPFL} $HTMLINDEX

rm -rf $TESTROOT

exit 0
