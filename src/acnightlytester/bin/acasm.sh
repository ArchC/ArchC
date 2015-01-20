#!/bin/bash

# This function is used to build binutils tools for a given model, testing ACASM, ArchC's binary tools synthetizer.
acasm_build_binary_tools() {
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

# This function is used to build binutils *ORIGINAL* tools for a given architecture, so we can compare and expect the results from these tools to be correct and
# validate ArchC's generated ones.
acasm_build_original_toolchain() {
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

#######################################
### RUN TESTS
#######################################

# This function runs ACASM validation script, assembling and linking a series of .s files using both the generated assembler and a correct reference
# assembler (binutils for a given architecture). The results are compared between these two assemblers and validation is successful if they have no
# difference.
acasm_run() {
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

#######################################
### TEST DRIVER
#######################################

acasm_test() {

    echo -ne "<h3>Testing: ACASM </h3>\n" >> $HTMLLOG
    echo -ne "<p>Command used to build ACASM: <b> acbingen.sh -a model -f model.ac </b> </p>\n" >> $HTMLLOG
    
    echo -ne "\n**********************************************\n"
    echo -ne "* Testing acasm                             **\n"
    echo -ne "**********************************************\n"

    if [ "$RUN_ARM_ACASM" != "no" ]; then
      acasm_build_binary_tools "arm"
      acasm_build_original_toolchain "arm" "arm"
    fi
    if [ "$RUN_SPARC_ACASM" != "no" ]; then
      acasm_build_binary_tools "sparc"
      acasm_build_original_toolchain "sparc" "sparc"
    fi
    if [ "$RUN_MIPS_ACASM" != "no" ]; then
      acasm_build_binary_tools "mips"
      acasm_build_original_toolchain "mips" "mips"
    fi
    if [ "$RUN_POWERPC_ACASM" != "no" ]; then
      acasm_build_binary_tools "powerpc"
      acasm_build_original_toolchain "powerpc" "powerpc"
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
      acasm_run "arm" "arm"
    fi
    if [ "$RUN_MIPS_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for mips ArchC model...\n"
      acasm_run "mips" "mips"
    fi
    if [ "$RUN_SPARC_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for sparc ArchC model...\n"
      acasm_run "sparc" "sparc"
    fi
    if [ "$RUN_POWERPC_ACASM" != "no" ]; then
      echo -ne "Validating binary tools generated for powerpc ArchC model...\n"
      acasm_run "powerpc" "powerpc"
    fi
    
    finalize_html $HTMLLOG "</table></p>"
}

