#!/bin/bash

acsim_build_model() {
      MODELNAME=$1
      MODELREV=$2
      USEACSIM=$3
      LOCAL_PARAMS=$4  # Each test have a specific set of params
      DIRSIMULATOR=$5     # Each test have a specific dir (e.g. arm/acsim, arm/accsim, arm/acstone)
    
      BUILD_FAULT="no"  # funcion return

      BUILD_RETCODE="false"
    if [ "$USEACSIM" != "no" ]; then    
        cd ${TESTROOT}/${MODELNAME}
        cp -r base $DIRSIMULATOR
        cd $DIRSIMULATOR
        TEMPFL=${RANDOM}.out
        if [ $LOCALSIMULATOR != "no" ]; then
            echo -ne "\n Building ${MODELNAME} ArchC model from a local source simulator..."
            if [ -e Makefile.archc ]; then
                make -f Makefile.archc clean &> /dev/null
                make -f Makefile.archc >> $TEMPFL 2>&1
            else
                echo -ne "<p><b><font color=\"crimson\">${MODELNAME} Makefile.archc not found; necessary when LOCALSIMULATOR=yes. Check script parameters.</font></b></p>\n" >> $HTMLLOG
                finalize_html $HTMLLOG ""
                echo -ne "Local simulator \e[31mfailed\e[m. Makefile.archc not found; necessary when LOCALSIMULATOR=yes. Check script parameters.\n"
                do_abort
           fi 
        else
            echo -ne "\n Building ${MODELNAME} ArchC Model with [ ${LOCAL_PARAMS} ] params..."
            if [ -e Makefile.archc ]; then
                make -f Makefile.archc distclean &> /dev/null
            fi
            ${TESTROOT}/install/bin/acsim ${MODELNAME}.ac ${LOCAL_PARAMS} > $TEMPFL 2>&1 && make -f Makefile.archc >> $TEMPFL 2>&1  
        fi
        BUILD_RETCODE=$?
        HTMLBUILDLOG=${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}-build-log.htm
        initialize_html $HTMLBUILDLOG "${MODELNAME} rev $MODELREV build output"
        format_html_output $TEMPFL $HTMLBUILDLOG
        finalize_html $HTMLBUILDLOG ""
        rm $TEMPFL

        if [ $BUILD_RETCODE -ne 0 ]; then
            echo -ne "<td><b><font color="crimson"> Failed </font></b>(<a href=\"${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}-build-log.htm\">log</a>)</td><td>-</td></th>" >> $HTMLLOG
            echo -ne "ACSIM \e[31mfailed\e[m to build $MODELNAME model.\n"
            BUILD_FAULT="yes"
        else
            echo -ne "<td><b><font color="green"> OK </font></b>(<a href=\"${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}-build-log.htm\">log</a>)</td>" >> $HTMLLOG
        fi
    fi
}

acsim_run() {
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
  export RUNLARGE   # 
  export COMPILE    # Definition in nightlytester.conf
  export RUNTEST    #
  export RUNTRAIN   # ================================== 
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
  export GCC  
  export MCF 
  export GOBMK    
  export HMMER      
  export SJENG 	    
  export LIBQUANTUM
  export H264        
  export OMNETPP    
  export ASTAR      

  export ENDIAN

  export TESTROOT
  export TESTCOMPILER
  export TESTCOMPILERCXX
  export TESTAR
  export TESTRANLIB
  export TESTFLAG

  export -f is_spec2006_enabled

  cd ${TESTROOT}/acsim
  ./acsim_validation.sh
  
  FAILED=`grep -ne "Failed" ${LOGROOT}/${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}.htm`

  if [ -z "$FAILED" ]; then
      echo -ne "<td><b><font color="green"> OK </font></b>(<a href=\"${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}.htm\">Report</a>) </td></tr>\n" >> $HTMLLOG
  else
      echo -ne "<td><b><font color="crimson"> Failed </font></b>(<a href=\"${HTMLPREFIX}-${MODELNAME}-${DIRSIMULATOR}.htm\">Report</a>)</td></tr>\n" >> $HTMLLOG
  fi
   


}

acsim_test(){
    echo -ne "<h3>Testing: ACSIM </h3>\n" >> $HTMLLOG
    if [ "$LOCALSIMULATOR" != "no" ]; then
        echo -ne "<p>Testing a Local Simulator. The 'acsim' was not run</p>\n" >> $HTMLLOG

        echo -ne "\n****************************************\n"
        echo -ne "* Testing Local Simulator             **\n"
        echo -ne "****************************************\n"
    else
        echo -ne "<p>Command used to build ACSIM models: <b> ./acsim model.ac ${ACSIM_PARAMS} </b> </p>\n" >> $HTMLLOG

        echo -ne "\n****************************************\n"
        echo -ne "* Testing ACSIM                       **\n"
        echo -ne "****************************************\n"
    fi

    cp ${SCRIPTROOT}/bin/acsim_validation.sh ${TESTROOT}/acsim/acsim_validation.sh
    chmod u+x ${TESTROOT}/acsim/acsim_validation.sh

    export LOGROOT
    export HTMLPREFIX

    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Compilation</th><th>Benchmark</th></tr>\n" >> $HTMLLOG

    if [ "$RUN_ARM_ACSIM" != "no" ]; then
        echo -ne "<tr><td>arm </td><td>${ARMREV}</td>" >> $HTMLLOG
        acsim_build_model "arm" "${ARMREV}" "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS}" "acsim" 
        echo -ne "\n Running ARM... \n"
        export TESTCOMPILER=$CROSS_ARM/`ls $CROSS_ARM | grep gcc$` 
        export TESTCOMPILERCXX=$CROSS_ARM/`ls $CROSS_ARM | grep g++$` 
        export TESTAR=$CROSS_ARM/`ls $CROSS_ARM | grep "\-ar$" | grep -v gcc` 
        export TESTRANLIB=$CROSS_ARM/`ls $CROSS_ARM | grep ranlib$ | grep -v gcc`
        export TESTFLAG=$CROSS_ARM_FLAG
        export ENDIAN="little" 
        acsim_run "arm" "${TESTROOT}/acsim/ARMMibench" "${TESTROOT}/acsim/ARMSpec" "${ARMREV}" "acsim" 
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
        echo -ne "<tr><td>sparc </td><td>${SPARCREV}</td>" >> $HTMLLOG
        acsim_build_model "sparc" "${SPARCREV}" "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS}" "acsim"
        echo -ne "\n Running Sparc... \n"
        export TESTCOMPILER=$CROSS_SPARC/`ls $CROSS_SPARC | grep gcc$` 
        export TESTCOMPILERCXX=$CROSS_SPARC/`ls $CROSS_SPARC | grep g++$` 
        export TESTAR=$CROSS_SPARC/`ls $CROSS_SPARC | grep "\-ar$" | grep -v gcc` 
        export TESTRANLIB=$CROSS_SPARC/`ls $CROSS_SPARC | grep ranlib$ | grep -v gcc`
        export TESTFLAG=$CROSS_SPARC_FLAG
        export ENDIAN="big" 
        acsim_run "sparc" "${TESTROOT}/acsim/SparcMibench" "${TESTROOT}/acsim/SparcSpec" "${SPARCREV}" "acsim" 
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
        echo -ne "<tr><td>mips </td><td>${MIPSREV}</td>" >> $HTMLLOG
        acsim_build_model "mips" "${MIPSREV}" "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS}" "acsim"
        echo -ne "\n Running Mips... \n"
        export TESTCOMPILER=$CROSS_MIPS/`ls $CROSS_MIPS | grep gcc$` 
        export TESTCOMPILERCXX=$CROSS_MIPS/`ls $CROSS_MIPS | grep g++$` 
        export TESTAR=$CROSS_MIPS/`ls $CROSS_MIPS | grep "\-ar$" | grep -v gcc` 
        export TESTRANLIB=$CROSS_MIPS/`ls $CROSS_MIPS | grep ranlib$ | grep -v gcc`
        export TESTFLAG=$CROSS_MIPS_FLAG
        export ENDIAN="big" 
        acsim_run "mips" "${TESTROOT}/acsim/MipsMibench" "${TESTROOT}/acsim/MipsSpec" "${MIPSREV}" "acsim" 
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
        echo -ne "<tr><td>powerpc </td><td>${PPCREV}</td>" >> $HTMLLOG
        acsim_build_model "powerpc" "${PPCREV}" "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS}" "acsim"
        echo -ne "\n Running PowerPC... \n"
        export TESTCOMPILER=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep gcc$` 
        export TESTCOMPILERCXX=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep g++$` 
        export TESTAR=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep "\-ar$" | grep -v gcc` 
        export TESTRANLIB=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep ranlib$ | grep -v gcc`
        export TESTFLAG=$CROSS_POWERPC_FLAG
        export ENDIAN="big" 
        acsim_run "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${TESTROOT}/acsim/PowerPCSpec" "${PPCREV}" "acsim" 
    fi

    finalize_html $HTMLLOG "</table></p>"
}
