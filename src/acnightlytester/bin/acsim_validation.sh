#!/bin/bash

## This script simulates Mibench programs using a defined ArchC simulator and compares the results with a golden model.
## This script was modified from its original version to adapt to ArchC nightlytester script (nightlytester.sh).
## Specifically, some internal parameters from this script are now defined and exported directly by nightlytester.sh when
## invoking this script to perform a specific platform validation.



# Script para validacão automatizada de um modelo ArchC com Mibench, baseado na comparacão com resultados de um "golden model".
# Instrucões de uso:
#
# 1. Todos os makefiles dos programas do benchmark devem estar configurados para compilar utilizando o cross compiler e flag para
# utilizar a newlib corretamente (por exemplo com a flag "-specs=ac_specs"). Opcionalmente, os Makefiles podem usar a variável
# $(TESTCOMPILER) para compilar, que é exportada por este script. Isto garante flexibilidade ao Makefile (não fica dependente de compilador).
# 2. Todos os scripts "runme_xxxxx.sh" devem estar configurados para executar o programa utilizando o simulador ArchC (por exemplo,
# adicionando o nome do executável do simulador antes do nome do programa, no script, como "armv5e.x --load=programa..." ao invés de "programa...")
# Opcionalmente, podem usar a variável de ambiente ${SIMULATOR}, que é exportada por este script. Isto garante flexibilidade aos scripts,
# pois o simulador pode ser escolhido em tempo de execução.

# Parâmetros que podem ser definidos por variáveis de ambiente
#[ -z "$TESTCOMPILER" ] && {
#  TESTCOMPILER="arm-elf-gcc -specs=ac_specs -static"
#  export TESTCOMPILER
#}
## nota: usado apenas no Makefile do JPEG
#[ -z "$TESTAR" ] && {
#  TESTAR="arm-elf-ar"
#  export TESTAR
#}
## nota: usado apenas no Makefile do JPEG
#[ -z "$TESTRANLIB" ] && {
#  TESTRANLIB="arm-elf-ranlib"
#  export TESTRANLIB
#}
#[ -z "$SIMULATOR" ] && {
#  SIMULATOR="armv5e.x --load="
#  export SIMULATOR
#}
[ -z "$ARCH" ] && {
  ARCH="armv5e"
}
# Parâmetros internos
##    commented out parameters are defined by nightlytester.sh when invoking this script for a specific test purpose
#GOLDENROOT=/home/rafael/disco2/rafael/valida/GoldenMibench  
#BENCHROOT=/home/rafael/disco2/rafael/valida/TestMibench
#STATSROOT=${BENCHROOT}/stats
#RUNSMALL=yes
#RUNLARGE=yes
#COMPILE=yes
#LOGROOT=/home/rafael/disco2/rafael/valida
#HTMLPREFIX=1 #This prefix is used to avoid name collisions between multiple html logs and build a coherent information repository
#DIFF="diff --report-identical-file --brief"
DIFF="diff -w"

aplicafiltro() {
	#${FILTRO} $1 > ${1}.temp
	#mv ${1}.temp $1
	return
}

# Param1 is the HTML file name
# Param2 is the "title" string
initialize_html() {
	echo -ne "<html> <head> <title> ${2} </title> </head><body>" > $1
	echo -ne "<h1>${2}</h1>" >> $1
}

# Param1 is the HTML file name
# Param2 is an string containing extra tag terminations (e.g. "</table>")
finalize_html() {
	echo -ne "$2</body>" >> $1
}

# Param1 is the input raw text to format
# Param2 is the HTML file name
format_html_output() {
	echo -ne "<table><tr><td><font face=\"Courier\">" >>$2
	sed -e 'a\<br\>' -e 's/ /\&nbsp;/g' <$1 1>>$2 2>/dev/null
	echo -ne "</font></td></tr></table>"  >>$2
}

# Param1 is the program name (to pring in log files!)
compile_mibench() {
	if [ "$COMPILE" != "no" ]; then
		HTML_COMP=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${1}-comp.htm
		initialize_html $HTML_COMP "${1} compilation results"
		TEMPFL=${RANDOM}.out
        echo -ne "Compiling...\n"
		make clean > /dev/null 
        make ENDIAN=${ENDIAN} > $TEMPFL 2>&1
        EXCODE=$?
        if [ $EXCODE -ne 0 ]; then
            echo -ne "<td><b><font color=\"crimson\"> failed </font></b>" >> $HTMLMAIN
        else
            echo -ne "<td><b><font color=\"green\"> OK </font></b>" >> $HTMLMAIN
        fi
        echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${1}-comp.htm\">log</a>)</td>" >> $HTMLMAIN
        format_html_output $TEMPFL $HTML_COMP
        finalize_html $HTML_COMP ""
        rm $TEMPFL		
    else
		echo -ne "<td><b><font color=\"fuchsia\"> N/A </font></b></td>" >> $HTMLMAIN
	fi
}

compile_spec(){
	if [ "$COMPILE" != "no" ]; then
		HTML_COMP=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${1}-comp.htm
		initialize_html $HTML_COMP "${1} compilation results"
		TEMPFL=${RANDOM}.out
        echo -ne "Compiling...\n"
        make SPEC=${SPECROOT} clean > /dev/null
        make SPEC=${SPECROOT} CC=${TESTCOMPILER} CXX=${TESTCOMPILERCXX} ENDIAN="${ENDIAN}" > $TEMPFL 2>&1
        EXCODE=$?
        if [ $EXCODE -ne 0 ]; then
            echo -ne "<td><b><font color=\"crimson\"> Failed </font></b>" >> $HTMLMAIN
        else
            echo -ne "<td><b><font color=\"green\"> OK </font></b>" >> $HTMLMAIN
        fi
        echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${1}-comp.htm\">log</a>)</td>" >> $HTMLMAIN
        format_html_output $TEMPFL $HTML_COMP
        finalize_html $HTML_COMP ""
        rm $TEMPFL		
	else
		echo -ne "<td><b><font color=\"fuchsia\"> N/A </font></b></td>" >> $HTMLMAIN
	fi

}

# This function should run a round of test with the configure simulator
# Param1 should inform the test script to run (e.g., "runme_small.sh")
# Param2 should inform the correct results folder (e.g., "${GOLDENROOT}/automotive/basicmath")
# Param3 is a string list of files to be compared (e.g., "output_small.txt output_small.inv.txt")
# Param4 should inform if we need to filter output produced by script (e.g. "no")
# Param5 is the condition variable telling wheeher we should really do the test (e.g. "${RUNSMALL}")
# Param6 is the name of the tested (to be printed in log files, e.g. "basicmath-large")
# e.g. run_test "runme_small.sh" "${GOLDENROOT}/automotive/basicmath" "output_small.txt" "yes" "$RUNSMALL" "basicmath-small"
run_test() {
	TESTSCRIPT=$1
	GOLDENRES=$2
	COMPLIST=$3
	RUNFILTER=$4
	CONDITION=$5
	TESTNAME=$6
	if [ "$CONDITION" != "no" ]; then
        for RESULTFILE in $COMPLIST 
		do
            # remove older RESULTFILE, if exist
            rm -f $RESULTFILE &> /dev/null
        done

		HTML_RUN=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-run.htm
		initialize_html $HTML_RUN "${TESTNAME} simulator output"
		echo -ne "Running script ${TESTSCRIPT}...\n"
		TEMPFL=${RANDOM}.out
		./${TESTSCRIPT} > $TEMPFL 2>&1
        if [ $DIRSIMULATOR == "powersc" ]; then
            mv window_power_report_${ARCH}.csv ${LOGROOT}/${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-win-pw-report.txt
        fi
		if [ "$COLLECT_STATS" != "no" ]; then
		  # Copy output to stats folder, to be later processed by collect_stats.py
		  cp $TEMPFL ${STATSROOT}/${TESTNAME}.${ARCH}.stats
		fi
		# Process output here!
		SIMSPEED=`sed -n -e '/Simulation speed/a\<br\>' -e '/Simulation speed/{s/.*Simulation speed: //;s/ /\&nbsp;/g;p}' <$TEMPFL`
		SIMSPEED=`sed -e 's/<br>$//' <<<$SIMSPEED`
		NUMINSTRS=`sed -n -e '/Number of instructions executed/a\<br\>' -e '/Number of instructions executed/{s/.*Number of instructions executed: //;s/ /\&nbsp;/g;p}' <$TEMPFL`
		NUMINSTRS=`sed -e 's/<br>$//' <<<$NUMINSTRS`
		format_html_output $TEMPFL $HTML_RUN
		finalize_html $HTML_RUN ""
		rm $TEMPFL
		HTML_DIFF=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-diff.htm
		initialize_html $HTML_DIFF "${TESTNAME} - output compared with golden model"
		DIFFERENCES=""
		for RESULTFILE in $COMPLIST 
		do
		  echo -ne "<h2>File ${RESULTFILE}</h2>" >> $HTML_DIFF
		  [ "$RUNFILTER" != "no" ] && {
			aplicafiltro ${RESULTFILE}
		  }
		  TEMPFL=${RANDOM}.out
		  ${DIFF} ${RESULTFILE} ${GOLDENRES}/${RESULTFILE} > $TEMPFL 2>&1
		  DIFFERENCES=$DIFFERENCES`cat $TEMPFL`
		  format_html_output $TEMPFL $HTML_DIFF
		  rm $TEMPFL
		done
		finalize_html $HTML_DIFF ""
		if [ -z "$DIFFERENCES" ]; then
		  echo -ne "<td><b><font color=\"green\"> OK </font></b>" >> $HTMLMAIN
                else
		  echo -ne "<td><b><font color=\"crimson\"> Failed </font></b>" >> $HTMLMAIN
		fi
        if [ $DIRSIMULATOR == "powersc" ]; then
    	    echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-run.htm\">simulator output</a>" >> $HTMLMAIN
    	    echo -ne ", <a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-diff.htm\">diff output</a>" >> $HTMLMAIN
            echo -ne ", <a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-win-pw-report.txt\">power report</a>)</td>" >> $HTMLMAIN
        else
    	    echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-run.htm\">simulator output</a>" >> $HTMLMAIN
            echo -ne ", <a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-${TESTNAME}-diff.htm\">diff output</a>)</td>" >> $HTMLMAIN
        fi
		# Printing simulation speed and number of instructions processed
		echo -ne "<td><b>$SIMSPEED</b></td>" >> $HTMLMAIN
		echo -ne "<td><b>$NUMINSTRS</b></td>" >> $HTMLMAIN
	else
		echo -ne "<td><b><font color=\"fuchsia\"> N/A </font></b></td>" >> $HTMLMAIN
		echo -ne "<td><b>-</b></td>" >> $HTMLMAIN
		echo -ne "<td><b>-</b></td>" >> $HTMLMAIN
	fi
}

# This function will sumarize statistical information about all ran programs using the collect_stats.py script.
build_stats() {
  HTMLSTATS=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-mibench-stats.htm
  initialize_html $HTMLSTATS "Mibench for ${ARCH} instruction usage data"
  cd ${STATSROOT}  
  python collect_stats.py ${ARCH} &> /dev/null
  format_html_output total.${ARCH}.stats $HTMLSTATS
  finalize_html $HTMLSTATS ""
  echo -ne "<p><B>Statistical information about instructions usage per category is available <a href=\"${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}-mibench-stats.htm\">here</a>.</B></p>\n" >> $HTMLMAIN
}

### Creating HTML's headers and general structure ###
HTMLMAIN=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${DIRSIMULATOR}.htm
echo -ne "<html> <head> <title> ${ARCH} Simulator - Mibench Results </title> </head><body>" > $HTMLMAIN
echo -ne "<h1>${ARCH} Simulator - Mibench Results</h1>" >> $HTMLMAIN
DATE=`date '+%a %D %r'`
echo -ne "<p>Produced by NightlyTester @ ${DATE}</p>"   >> $HTMLMAIN
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLMAIN

#echo -ne "<tr><th>Program</th><th>Program</th><th>Small</th><th></th><th></th><th>Large</th><th></th><th></th></tr>\n" >> $HTMLMAIN
echo -ne "<tr><th>MiBench</th><th>Compilation</th><th>Simulation (small)</th><th>Speed</th><th># Instrs.</th><th>Simulation (large)</th><th>Speed</th><th># Instrs.</th></tr>\n" >> $HTMLMAIN

#echo -ne "<tr><td>Automotive</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN
#basicmath
[ "$BASICMATH" != "no" ] && {
	echo -ne "\nCurrently testing: BASICMATH\n"
	echo -ne "<tr><td>basicmath</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/automotive/basicmath
	compile_mibench "basicmath"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/basicmath" "output_small.txt" "no" "$RUNSMALL" "basicmath-small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/basicmath" "output_large_softfloat.txt" "no" "$RUNLARGE" "basicmath-large"	
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#bitcount
[ "$BITCOUNT" != "no" ] && {
	echo -ne "\nCurrently testing: BITCOUNT\n"
	echo -ne "<tr><td>bitcount</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/automotive/bitcount
	compile_mibench "bitcount"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/bitcount" "output_small.txt" "yes" "$RUNSMALL" "bitcount-small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/bitcount" "output_large.txt" "yes" "$RUNLARGE" "bitcount-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


#qsort  -- problemas: lembrar de corrigir o fonte, que causa falha de segmentacao se MAXARRAY for grande! (sugestao corrigir para MAXARRAY=10000)
[ "$QUICKSORT" != "no" ] && {
	echo -ne "\nCurrently testing: QSORT\n"
	echo -ne "<tr><td>qsort</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/automotive/qsort
	compile_mibench "quicksort"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/qsort" "output_small.txt" "yes" "$RUNSMALL" "qsort-small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/qsort" "output_large.txt" "yes" "$RUNLARGE" "qsort-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


#susan
[ "$SUSAN" != "no" ] && {
	echo -ne "\nCurrently testing: SUSAN\n"
	echo -ne "<tr><td>susan</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/automotive/susan
	compile_mibench "susan"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/susan" "output_small.smoothing.pgm output_small.edges.pgm output_small.corners.pgm" "no" "$RUNSMALL" "susan-small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/susan" "output_large.smoothing.pgm output_large.edges.pgm output_large.corners.pgm" "no" "$RUNLARGE" "susan-large"	
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- telecomm ---
#echo -ne "<tr><td>Telecomm</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#adpcm
[ "$ADPCM" != "no" ] && {
	echo -ne "\nCurrently testing: ADPCM\n"
	echo -ne "<tr><td>adpcm</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/telecomm/adpcm/src
	compile_mibench "adpcm"
	cd ..
	chmod u+x *.sh
    if [ "$ENDIAN" == "big" ]; then
    	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/adpcm" "output_small.adpcm BIG_ENDIAN_output_small.pcm" "no" "$RUNSMALL" "adpcm-small"
	    run_test "runme_large.sh" "${GOLDENROOT}/telecomm/adpcm" "output_large.adpcm BIG_ENDIAN_output_large.pcm" "no" "$RUNLARGE" "adpcm-large"
    else
    	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/adpcm" "output_small.adpcm output_small.pcm" "no" "$RUNSMALL" "adpcm-small"
	    run_test "runme_large.sh" "${GOLDENROOT}/telecomm/adpcm" "output_large.adpcm output_large.pcm" "no" "$RUNLARGE" "adpcm-large"
    fi
	echo -ne "</tr>\n" >> $HTMLMAIN
}
#CRC32
[ "$CRC" != "no" ] && {
	echo -ne "\nCurrently testing: CRC32\n"
	echo -ne "<tr><td>crc32</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/telecomm/CRC32
	compile_mibench "crc"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/CRC32" "output_small.txt" "no" "$RUNSMALL" "crc32-small"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/CRC32" "output_large.txt" "no" "$RUNLARGE" "crc32-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
#FFT
[ "$FFT" != "no" ] && {
	echo -ne "\nCurrently testing: FFT\n"
	echo -ne "<tr><td>fft</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/telecomm/FFT
	compile_mibench "fft"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/FFT" "output_small.txt output_small.inv.txt" "no" "$RUNSMALL" "fft-small"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/FFT" "output_large.txt output_large.inv.txt" "no" "$RUNLARGE" "fft-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#gsm
[ "$GSM" != "no" ] && {
	echo -ne "\nCurrently testing: GSM\n"
	echo -ne "<tr><td>gsm</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/telecomm/gsm
	compile_mibench "gsm"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/gsm" "output_small.encode.gsm output_small.decode.run" "no" "$RUNSMALL" "gsm-small"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/gsm" "output_large.encode.gsm output_large.decode.run" "no" "$RUNLARGE" "gsm-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- network ---
#echo -ne "<tr><td>Network</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#dijkstra
[ "$DIJKSTRA" != "no" ] && {
	echo -ne "\nCurrently testing: DIJKSTRA\n"
	echo -ne "<tr><td>dijkstra</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/network/dijkstra
	compile_mibench "dijkstra"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/network/dijkstra" "output_small.dat" "no" "$RUNSMALL" "dijkstra-small"
	run_test "runme_large.sh" "${GOLDENROOT}/network/dijkstra" "output_large.dat" "no" "$RUNLARGE" "dijkstra-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#patricia
[ "$PATRICIA" != "no" ] && {
	echo -ne "\nCurrently testing: PATRICIA\n"
	echo -ne "<tr><td>patricia</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/network/patricia
	compile_mibench "patricia"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/network/patricia" "output_small.txt" "no" "$RUNSMALL" "patricia-small"
	run_test "runme_large.sh" "${GOLDENROOT}/network/patricia" "output_large.txt" "no" "$RUNLARGE" "patricia-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- security ---
#echo -ne "<tr><td>Security</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#rijndael
[ "$RIJNDAEL" != "no" ] && {
	echo -ne "\nCurrently testing: RIJNDAEL\n"
	echo -ne "<tr><td>rijndael</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/security/rijndael
	compile_mibench "rijndael"
	chmod u+x *.sh

    if [ "$ENDIAN" == "little" ]; then
	    run_test "runme_small.sh" "${GOLDENROOT}/security/rijndael" "LITTLE_ENDIAN_output_small.enc output_small.dec" "no" "$RUNSMALL" "rijndael-small"
    	run_test "runme_large.sh" "${GOLDENROOT}/security/rijndael" "LITTLE_ENDIAN_output_large.enc output_large.dec" "no" "$RUNLARGE" "rijndael-large"
    else
    	run_test "runme_small.sh" "${GOLDENROOT}/security/rijndael" "output_small.enc output_small.dec" "no" "$RUNSMALL" "rijndael-small"
	    run_test "runme_large.sh" "${GOLDENROOT}/security/rijndael" "output_large.enc output_large.dec" "no" "$RUNLARGE" "rijndael-large"
    fi
	echo -ne "</tr>\n" >> $HTMLMAIN
}	

#sha
[ "$SHA" != "no" ] && {
	echo -ne "\nCurrently testing: SHA\n"
	echo -ne "<tr><td>sha</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/security/sha
	compile_mibench "sha"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/security/sha" "output_small.txt" "yes" "$RUNSMALL" "sha-small"
	run_test "runme_large.sh" "${GOLDENROOT}/security/sha" "output_large.txt" "yes" "$RUNLARGE" "sha-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

# --- consumer ---
#echo -ne "<tr><td>Consumer</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#jpeg
[ "$JPEG" != "no" ] && {
	echo -ne "\nCurrently testing: JPEG\n"
	echo -ne "<tr><td>jpeg</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/consumer/jpeg/jpeg-6a
	compile_mibench "jpeg"
	cd ..
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/consumer/jpeg" "output_small_encode.jpeg output_small_decode.ppm" "no" "$RUNSMALL" "jpeg-small"
	run_test "runme_large.sh" "${GOLDENROOT}/consumer/jpeg" "output_large_encode.jpeg output_large_decode.ppm" "no" "$RUNLARGE" "jpeg-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#lame
[ "$LAME" != "no" ] && {
	echo -ne "\nCurrently testing: LAME\n"
	echo -ne "<tr><td>lame</td>" >> $HTMLMAIN
	cd ${MIBENCHROOT}/consumer/lame/lame3.70
	compile_mibench "lame"
	cd ..
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/consumer/lame" "output_small.mp3" "no" "$RUNSMALL" "lame-small"
	run_test "runme_large.sh" "${GOLDENROOT}/consumer/lame" "output_large.mp3" "no" "$RUNLARGE" "lame-large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


echo -ne "<tr><td colspan=8 height=25></td></tr>\n" >> $HTMLMAIN
# --- SPEC2006 ---
if is_spec2006_enabled; then
    echo -ne "<tr><th>SPEC2006</th><th>Compilation</th><th>Test Data Set</th><th>Speed</th><th># Instr.</th>
                                                                  <th>Train Data Set</th><th>Speed</th><th># Instr.</th></tr>\n" >> $HTMLMAIN
fi

[ "$BZIP_2" != "no" ] && {
	echo -ne "\nCurrently testing: 401.bzip2\n"
	echo -ne "<tr><td>401.bzip2</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/401.bzip2/src
    compile_spec "401.bzip2"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/401.bzip2/data/test/output" "input.program.out" "no" "$RUNTEST" "401.bzip2-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/401.bzip2/data/test/output" "input.combined.out" "no" "$RUNTRAIN"  "401.bzip2-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$GCC" != "no" ] && {
    APP="403.gcc" 
	echo -ne "\nCurrently testing: $APP\n"
	echo -ne "<tr><td>$APP</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/$APP/src
    compile_spec "$APP"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/$APP/data/test/output" "cccp.s" "no" "$RUNTEST" "$APP-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/$APP/data/test/output" "integrate.s" "no" "$RUNTRAIN"  "$APP-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


[ "$MCF" != "no" ] && {
	echo -ne "\nCurrently testing: 429.mcf\n"
	echo -ne "<tr><td>429.mcf</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/429.mcf/src
    compile_spec "429.mcf"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/429.mcf/data/test/output" "inp.out" "no" "$RUNTEST" "429.mcf-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/429.mcf/data/test/output" "inp.out" "no" "$RUNTRAIN"  "429.mcf-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$GOBMK" != "no" ] && {
	echo -ne "\nCurrently testing: 445.gobmk\n"
	echo -ne "<tr><td>445.gobmk</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/445.gobmk/src
    compile_spec "445.gobmk"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/445.gobmk/data/test/output" "capture.out" "no" "$RUNTEST" "445.gobmk-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/445.gobmk/data/test/output" "capture.out" "no" "$RUNTRAIN"  "445.gobmk-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$HMMER" != "no" ] && {
	echo -ne "\nCurrently testing: 456.hmmer\n"
	echo -ne "<tr><td>456.hmmer</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/456.hmmer/src
    compile_spec "456.hmmer"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/456.hmmer/data/test/output" "bombesin.out" "no" "$RUNTEST" "456.hmmer-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/456.hmmer/data/test/output" "bombesin.out" "no" "$RUNTRAIN"  "456.hmmer-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$SJENG" != "no" ] && {
	echo -ne "\nCurrently testing: 458.sjeng\n"
	echo -ne "<tr><td>458.sjeng</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/458.sjeng/src
    compile_spec "458.sjeng"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/458.sjeng/data/test/output" "test.out" "no" "$RUNTEST" "458.sjeng-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/458.sjeng/data/test/output" "test.out" "no" "$RUNTRAIN"  "458.sjeng-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$LIBQUANTUM" != "no" ] && {
	echo -ne "\nCurrently testing: 462.libquantum\n"
	echo -ne "<tr><td>462.libquantum</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/462.libquantum/src
    compile_spec "462.libquantum"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/462.libquantum/data/test/output" "test.out" "no" "$RUNTEST" "462.libquantum-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/462.libquantum/data/test/output" "test.out" "no" "$RUNTRAIN"  "462.libquantum-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$H264" != "no" ] && {
	echo -ne "\nCurrently testing: 464.h264ref\n"
	echo -ne "<tr><td>464.h264ref</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/464.h264ref/src
    compile_spec "464.h264ref"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/464.h264ref/data/test/output" "foreman_test_baseline_encodelog.out" "no" "$RUNTEST"  "464.h264ref-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/464.h264ref/data/test/output" "foreman_test_baseline_encodelog.out" "no" "$RUNTRAIN"   "464.h264ref-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$OMNETPP" != "no" ] && {
	echo -ne "\nCurrently testing: 471.omnetpp\n"
	echo -ne "<tr><td>471.omnetpp</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/471.omnetpp/src
    compile_spec "471.omnetpp"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/471.omnetpp/data/test/output" "omnetpp.log" "no" "$RUNTEST"  "471.omnetpp-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/471.omnetpp/data/test/output" "omnetpp.log" "no" "$RUNTRAIN"   "471.omnetpp-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

[ "$ASTAR" != "no" ] && {
	echo -ne "\nCurrently testing: 473.astar\n"
	echo -ne "<tr><td>473.astar</td>" >> $HTMLMAIN
	cd ${SPECROOT}/CPU2006/473.astar/src
    compile_spec "473.astar"
    cd ..
    chmod u+x *.sh
    run_test "runme_test.sh" "${GOLDENSPECROOT}/473.astar/data/test/output" "lake.out" "no" "$RUNTEST"  "473.astar-test"
    run_test "runme_train.sh" "${GOLDENSPECROOT}/473.astar/data/test/output" "lake.out" "no" "$RUNTRAIN"   "473.astar-test"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
echo -ne "</table>\n" >> $HTMLMAIN

# Collect statistical information 
if [ "$COLLECT_STATS" != "no" ]; then
  build_stats
fi

finalize_html $HTMLMAIN ""
