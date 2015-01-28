#!/bin/bash

# NightlyTester script for ArchC.
# Clone ArchC source in GIT and tests current version
#
# ArchC Team

# Parameters adjustable by environment variables

NIGHTLYVERSION=2.1

####################################
### Import external funtions
####################################
. bin/helper_functions.sh
. bin/acsim.sh
. bin/powersc.sh

####################################
### ENTRY POINT
####################################

# Check if other instance of Nightly is running (LOCK file)
if [ -a /tmp/nightly-token ]; then
    echo -ne "A instance of Nightly is running...\n"
    exit 0
else
    touch /tmp/nightly-token
fi

# Initializing HTML log files
# Discover this run's number and prefix all our HTML files with it

if [ ! -a $HTMLINDEX ]; then
    cp htmllogs/index.htm $HTMLINDEX
fi
export HTMLPREFIX=`sed -n -e '/<tr><td>[0-9]\+/{s/<tr><td>\([0-9]\+\).*/\1/;p;q}' <${HTMLINDEX}`
export LASTHTMLPREFIX=$HTMLPREFIX
export LASTARCHCREV=`grep -e "<tr><td>" < ${HTMLINDEX} | head -n 1 | cut -d\> -f 7 | cut -d\< -f 1`

if [ -z $LASTARCHCREV ]; then
    echo -ne "Problem in index.html file.\n"
    do_abort
fi

export LASTEQCURRENT="yes"

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
  LASTEQCURRENT="no"
else
  echo -ne "<tr><td>ArchC</td><td>${CLONELINK}</td></tr>\n" >> $HTMLLOG
fi

if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_ARM_ACASM" != "no" -o "$RUN_ARM_ACCSIM" != "no" ]; then
  if [ -z "$ARMGITLINK" ]; then
    echo -ne "<tr><td>ARM Model</td><td>${ARMWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="no"
  else
    echo -ne "<tr><td>ARM Model</td><td>${ARMGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_MIPS_ACSIM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" ]; then
  if [ -z "$MIPSGITLINK" ]; then
    echo -ne "<tr><td>MIPS Model</td><td>${MIPSWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="no"
  else
    echo -ne "<tr><td>MIPS Model</td><td>${MIPSGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_SPARC_ACSIM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" ]; then
  if [ -z "$SPARCGITLINK" ]; then
    echo -ne "<tr><td>SPARC Model</td><td>${SPARCWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="no"
  else
   echo -ne "<tr><td>SPARC Model</td><td>${SPARCGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
if [ "$RUN_POWERPC_ACSIM" != "no" -o "$RUN_POWERPC_ACASM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
   if [ -z "$POWERPCGITLINK" ]; then
    echo -ne "<tr><td>POWERPC Model</td><td>${POWERPCWORKINGCOPY}</td></tr>\n" >> $HTMLLOG
    LASTEQCURRENT="no"
  else
    echo -ne "<tr><td>POWERPC Model</td><td>${POWERPCGITLINK}</td></tr>\n" >> $HTMLLOG
  fi
fi
echo -ne "</table></p>\n" >> $HTMLLOG


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
        LASTEQCURRENT="no"
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

if [ "$SYSTEMCCOMPILE" != "yes" ]; then
  echo -ne "<p>User-supplied SystemC path: ${SYSTEMCPATH}</p>\n" >> $HTMLLOG
fi

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
./configure --prefix=${TESTROOT}/install $ACSIM_STRING $ACASM_STRING $ACSTONE_STRING >> $TEMPFL 2>&1    

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
if [ "$FORCENIGHTLY" != "yes" -a "$LASTEQCURRENT" == "yes" ]; then
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
    tar -xjf ${SCRIPTROOT}/sources/SourceMibench.tar.bz2
    [ $? -ne 0 ] && do_abort
    mv SourceMibench ARMMibench
    if is_spec2006_enabled; then
        echo -ne "Uncompressing SPEC2006 from source to ARM cross compiling...\n"
        #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
        cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
        [ $? -ne 0 ] && do_abort
        mv SourceSPEC2006 ARMSpec
    fi
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
     tar -xjf ${SCRIPTROOT}/sources/SourceMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceMibench SparcMibench
     if is_spec2006_enabled; then
         echo -ne "Uncompressing SPEC2006 from source to SPARC cross compiling...\n"
         #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
         cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
         [ $? -ne 0 ] && do_abort
         mv SourceSPEC2006 SparcSpec
     fi
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
     tar -xjf ${SCRIPTROOT}/sources/SourceMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceMibench MipsMibench
     if is_spec2006_enabled; then
         echo -ne "Uncompressing SPEC2006 from source to MIPS cross compiling...\n"
         #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
         cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
         [ $? -ne 0 ] && do_abort
         mv SourceSPEC2006 MipsSpec
     fi
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
     tar -xjf ${SCRIPTROOT}/sources/SourceMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
     mv SourceMibench PowerPCMibench
     if is_spec2006_enabled; then
        echo -ne "Uncompressing SPEC2006 from source to POWERPC cross compiling...\n"
        #tar -xjf ${SCRIPTROOT}/sources/SourceSPEC2006.tar.bz2
        cp -r ${SCRIPTROOT}/sources/SourceSPEC2006 ${TESTROOT}/acsim
        [ $? -ne 0 ] && do_abort
        mv SourceSPEC2006 PowerPCSpec
    fi
  else 
     echo -ne "Uncompressing Mibench precompiled for PowerPC...\n"
     cd ${TESTROOT}/acsim
     tar -xjf ${SCRIPTROOT}/sources/PowerPCMibench.tar.bz2
     [ $? -ne 0 ] && do_abort
  fi
fi

if [ "$LOCALSIMULATOR" != "no" ]; then

    # FIXME: Local Simulator may be any class of simulator, not only acsim class
    acsim_test

    finalize_nightly_tester
    exit 0
fi

if [ "$RUN_ARM_ACSIM" != "no" -o "$RUN_MIPS_ACSIM" != "no" -o "$RUN_SPARC_ACSIM" != "no" -o "$RUN_POWERPC_ACSIM" != "no" ]; then
    acsim_test
fi

if [ $RUN_POWERSC != "no" ]; then
    powersc_test
fi

if [ "$RUN_ARM_ACCSIM" != "no" -o "$RUN_MIPS_ACCSIM" != "no" -o "$RUN_SPARC_ACCSIM" != "no" -o "$RUN_POWERPC_ACCSIM" != "no" ]; then
    accsim_test
fi

if [ "$RUN_ARM_ACASM" != "no" -o "$RUN_MIPS_ACASM" != "no" -o "$RUN_SPARC_ACASM" != "no" -o "$RUN_POWERPC_ACASM" != "no" ]; then
    acasm_test
fi

if [ $RUN_ACSTONE != "no" ]; then
    acstone_test
fi


#########################

finalize_nightly_tester

exit 0
