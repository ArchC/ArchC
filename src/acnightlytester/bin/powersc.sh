#!/bin/bash

##########################################
### Build and Run on 'acsim.sh'
##########################################


#######################################
### TEST DRIVER
#######################################

powersc_test() {
 
    echo -ne "<h3>Testing: POWERSC </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build PowerSC models: <b> ./acsim model.ac ${ACSIM_PARAMS} -pw </b> </p>\n" >> $HTMLLOG
    echo -ne "<p> <b>Note: </b> ARM and PowerPC models don't have POWERSC table files.</p>\n" >> $HTMLLOG 

    echo -ne "\n**********************************************\n"
    echo -ne "* Testing PowerSC                           **\n"
    echo -ne "**********************************************\n"

    # ARM and PowerPC dont have POWERSC table files
    ARM_POWERSC="no"
    POWERPC_POWERSC="no"

    SPARC_POWERSC=$RUN_SPARC_ACSIM
    MIPS_POWERSC=$RUN_MIPS_ACSIM

    cp ${SCRIPTROOT}/bin/acsim_validation.sh ${TESTROOT}/acsim/acsim_validation.sh
    chmod u+x ${TESTROOT}/acsim/acsim_validation.sh
    export LOGROOT
    export HTMLPREFIX
    
    echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLLOG
    echo -ne "<tr><th>Component</th><th>Version</th><th>Compilation</th><th>Benchmark</th></tr>\n" >> $HTMLLOG

    if [ "$ARM_POWERSC" != "no" ]; then
        echo -ne "<tr><td>arm </td><td>${ARMREV}</td>" >> $HTMLLOG
        acsim_build_model "arm" "${ARMREV}" "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        if [ ${BUILD_FAULT} != "yes" ]; then
            echo -ne "\n Running ARM... \n"
            export TESTCOMPILER=$CROSS_ARM/`ls $CROSS_ARM | grep gcc$` 
            export TESTCOMPILERCXX=$CROSS_ARM/`ls $CROSS_ARM | grep g++$` 
            export TESTAR=$CROSS_ARM/`ls $CROSS_ARM | grep "\-ar$" | grep -v gcc` 
            export TESTRANLIB=$CROSS_ARM/`ls $CROSS_ARM | grep ranlib$ | grep -v gcc`
            export TESTFLAG=$CROSS_ARM_FLAG
            export ENDIAN="little" 
            acsim_run "arm" "${TESTROOT}/acsim/ARMMibench" "${TESTROOT}/acsim/ARMSpec" "${ARMREV}" "powersc"
        fi
    fi
    if [ "$SPARC_POWERSC" != "no" ]; then
        echo -ne "<tr><td>sparc </td><td>${SPARCREV}</td>" >> $HTMLLOG
        acsim_build_model "sparc" "${SPARCREV}"  "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        if [ ${BUILD_FAULT} != "yes" ]; then
            echo -ne "\n Running Sparc... \n"
            export TESTCOMPILER=$CROSS_SPARC/`ls $CROSS_SPARC | grep gcc$` 
            export TESTCOMPILERCXX=$CROSS_SPARC/`ls $CROSS_SPARC | grep g++$` 
            export TESTAR=$CROSS_SPARC/`ls $CROSS_SPARC | grep "\-ar$" | grep -v gcc` 
            export TESTRANLIB=$CROSS_SPARC/`ls $CROSS_SPARC | grep ranlib$ | grep -v gcc`
            export TESTFLAG=$CROSS_SPARC_FLAG
            export ENDIAN="big" 
            acsim_run "sparc" "${TESTROOT}/acsim/SparcMibench" "${TESTROOT}/acsim/SparcSpec" "${SPARCREV}" "powersc"
        fi
    fi
    if [ "$MIPS_POWERSC" != "no" ]; then
        echo -ne "<tr><td>mips </td><td>${MIPSREV}</td>" >> $HTMLLOG
        acsim_build_model "mips" "${MIPSREV}"  "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        if [ ${BUILD_FAULT} != "yes" ]; then
            echo -ne "\n Running Mips... \n"
            export TESTCOMPILER=$CROSS_MIPS/`ls $CROSS_MIPS | grep gcc$` 
            export TESTCOMPILERCXX=$CROSS_MIPS/`ls $CROSS_MIPS | grep g++$` 
            export TESTAR=$CROSS_MIPS/`ls $CROSS_MIPS | grep "\-ar$" | grep -v gcc` 
            export TESTRANLIB=$CROSS_MIPS/`ls $CROSS_MIPS | grep ranlib$ | grep -v gcc`
            export TESTFLAG=$CROSS_MIPS_FLAG
            export ENDIAN="big" 
            acsim_run "mips" "${TESTROOT}/acsim/MipsMibench" "${TESTROOT}/acsim/MipsSpec" "${MIPSREV}" "powersc"
        fi
    fi
    if [ "$POWERPC_POWERSC" != "no" ]; then
        echo -ne "<tr><td>powerpc </td><td>${PPCREV}</td>" >> $HTMLLOG
        acsim_build_model "powerpc" "${PPCREV}" "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS} -pw" "powersc"
        if [ ${BUILD_FAULT} != "yes" ]; then
            echo -ne "\n Running PowerPC... \n"
            export TESTCOMPILER=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep gcc$` 
            export TESTCOMPILERCXX=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep g++$` 
            export TESTAR=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep "\-ar$" | grep -v gcc` 
            export TESTRANLIB=$CROSS_POWERPC/`ls $CROSS_POWERPC | grep ranlib$ | grep -v gcc`
            export TESTFLAG=$CROSS_POWERPC_FLAG
            export ENDIAN="big" 
           acsim_run "powerpc" "${TESTROOT}/acsim/PowerPCMibench" "${TESTROOT}/acsim/PowerPCSpec" "${PPCREV}" "powersc"
        fi
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

