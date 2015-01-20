#!/bin/bash

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

#######################################
### RUN TESTS
#######################################

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
    cp ${SCRIPTROOT}/bin/acstone_run_all.sh ${TESTROOT}/acstone &&
      cp ${SCRIPTROOT}/bin/acstone_run_teste.sh ${TESTROOT}/acstone &&
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
      build_model "arm" "${ARMREV}"  "${RUN_ARM_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "arm"
      run_tests_acsim_acstone "arm" "${ARMREV}"
    fi
    if [ "$RUN_SPARC_ACSIM" != "no" ]; then
      build_model "sparc" "${SPARCREV}"  "${RUN_SPARC_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "sparc"
      run_tests_acsim_acstone "sparc" "${SPARCREV}"
    fi
    if [ "$RUN_MIPS_ACSIM" != "no" ]; then
      build_model "mips" "${MIPSREV}" "${RUN_MIPS_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "mips"
      run_tests_acsim_acstone "mips" "${MIPSREV}"
    fi
    if [ "$RUN_POWERPC_ACSIM" != "no" ]; then
      build_model "powerpc" "${PPCREV}"  "${RUN_POWERPC_ACSIM}" "${ACSIM_PARAMS} -gdb" "acstone" 
      build_gdb "powerpc"
      run_tests_acsim_acstone "powerpc" "${PPCREV}"    
    fi

    finalize_html $HTMLLOG "</table></p>"
}

