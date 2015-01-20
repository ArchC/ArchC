#!/bin/bash

#######################################
### RUN TESTS
#######################################

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
  export RUNLARGE   # 
  export COMPILE    # Definition in nightlytester.conf
  export RUNTEST    #
  export RUNTRAIN   # ================================== 
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

test_accsim() {

    echo -ne "<h3>Testing: ACCSIM </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build ACCSIM models: <b> ./accsim model.ac ${ACCSIM_PARAMS} /path/to/program </b> </p>\n" >> $HTMLLOG

    echo -ne "\n****************************************\n"
    echo -ne "* Testing ACCSIM                       **\n"
    echo -ne "*****************************************\n"

    cp ${SCRIPTROOT}/bin/validation-accsim.sh ${TESTROOT}/acsim/validation-accsim.sh
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

