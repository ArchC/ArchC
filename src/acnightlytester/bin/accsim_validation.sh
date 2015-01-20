#!/bin/bash

## This script was adapted to test accsim, the ArchC compiled simulator,
## using Mibench programs.

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
[ -z "$TESTCOMPILER" ] && {
  TESTCOMPILER="arm-elf-gcc -specs=ac_specs -static"
  export TESTCOMPILER
}
# nota: usado apenas no Makefile do JPEG
[ -z "$TESTAR" ] && {
  TESTAR="arm-elf-ar"
  export TESTAR
}
# nota: usado apenas no Makefile do JPEG
[ -z "$TESTRANLIB" ] && {
  TESTRANLIB="arm-elf-ranlib"
  export TESTRANLIB
}
[ -z "$SIMULATOR" ] && {
  SIMULATOR="armv5e.x"
  export SIMULATOR
}
[ -z "$ARCH" ] && {
  ARCH="armv5e"
}
# Parâmetros internos
##    commented out parameters are defined by nightlytester.sh when invoking this script for a specific test purpose
#GOLDENROOT=/home/rafael/disco2/rafael/valida/GoldenMibench  
#BENCHROOT=/home/rafael/disco2/rafael/valida/TestMibench
#STATSROOT=${BENCHROOT}/stats
#SIMROOT=/home/rafael/disco2/rafael/valida/install
#TOOLSPATH=/home/rafael/disco2/rafael/install
#ACCSIMFLAGS=-ai
#RUNSMALL=yes
#RUNLARGE=yes
#COMPILE=yes
#LOGROOT=/home/rafael/disco2/rafael/valida
#HTMLPREFIX=1 #This prefix is used to avoid name collisions between multiple html logs and build a coherent information repository
#DIFF="diff --report-identical-file --brief"
DIFF="diff"

#programas
#BASICMATH=yes # demorado
#BITCOUNT=yes
#QUICKSORT=yes
#SUSAN=yes
#ADPCM=yes
#CRC=yes
#FFT=yes # demorado
#GSM=yes
#DIJKSTRA=yes
#PATRICIA=yes # demorado
#RIJNDAEL=yes
#SHA=yes
#JPEG=yes
#LAME=yes # demorado
#filtro para remover o aviso do SystemC dos arquivos de saída
#Apenas deverá ser usado se o SystemC produz um aviso "deprecated"
#quando o simulador é executado.
FILTRO="sed 1,2d"
# Script

aplicafiltro() {
        # not used for accsim...
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
compile_prog() {
	if [ "$COMPILE" != "no" ]; then
		HTML_COMP=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${1}-comp.htm
		initialize_html $HTML_COMP "${1} compilation results"
		TEMPFL=${random}.out
		make clean 
		make > $TEMPFL 2>&1
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
# Param5 is the condition variable telling whether we should really do the test (e.g. "${RUNSMALL}")
# Param6 is the name of the tested (to be printed in log files, e.g. "basicmath-large")
# Param7 is the path root to the program to be tested (e.g. "${BENCHROOT}/automotive/basicmath")
# Param8 is a string list of executable files to be precompiled (e.g. "basicmath")
# e.g. run_test "runme_small.sh" "${GOLDENROOT}/automotive/basicmath" "output_small.txt" "yes" "$RUNSMALL" "basicmath-small" "${BENCHROOT}/automotive/basicmath" "basicmath_small"
run_test() {
	TESTSCRIPT=$1
	GOLDENRES=$2
	COMPLIST=$3
	RUNFILTER=$4
	CONDITION=$5
	TESTNAME=$6
	PROGROOT=$7
	PROGNAMES=$8
	if [ "$CONDITION" != "no" ]; then
	        # Pre-compiling this simulation with accsim	        
		echo -ne "Processing ${TESTSCRIPT}...\n"
		TOTALERRORS=0
		COMPOUT=${SIMROOT}/${RANDOM}.out
		SIMOUT=${RANDOM}.sim
		DUMMYSCRIPT=${RANDOM}.sh
		for OBJFILE in $PROGNAMES
		do
		  echo -ne "Simulation compilation output for ${OBJFILE}\n" >> $COMPOUT
		  cd ${SIMROOT}
		  echo -ne "Compiling simulation for ${OBJFILE}...\n"
		  make -f Makefile.archc distclean &> /dev/null 
		  ${TOOLSPATH}/accsim ${ARCH}.ac ${ACCSIMFLAGS} ${PROGROOT}/${OBJFILE} >> $COMPOUT 2>&1
		  EXCODE=$?
		  if [ $EXCODE -eq 0 ]; then
		    make -f Makefile.archc >> $COMPOUT 2>&1
		    EXCODE=$?
		  fi
		  if [ $EXCODE -ne 0 ]; then
		    TOTALERRORS=$(($TOTALERRORS + 1))
		    echo -ne "Simulation compilation failed.\n"
		  else
		    cd ${PROGROOT}
		    # Extracting  parameters from acsim mibench test scripts
		    #RUNPARAMS=`sed -n -e "/^\\${SIMULATOR}/{s/^\\${SIMULATOR}${OBJFILE}//;p}" < ${TESTSCRIPT}`
		    OBJFILECONV=`sed -e 's/\//\\\\\//' <<< $OBJFILE`
		    #SIMROOTCONV=`sed -e 's/\//\\\\\//' <<< $SIMROOT`
		    echo -ne "#!/bin/bash\n" > $DUMMYSCRIPT
		    sed -n -e "/^\${SIMULATOR}${OBJFILECONV}/{s,^\${SIMULATOR}${OBJFILE},,;p}" < ${TESTSCRIPT} | sed -e "s,\(.*\),${SIMROOT}\/${ARCH}.x\0," >> $DUMMYSCRIPT
		    #echo -ne "${SIMROOT}/${ARCH}.x ${RUNPARAMS}\n" >> $DUMMYSCRIPT
		    chmod u+x $DUMMYSCRIPT
		    echo -ne "Running simulation for ${OBJFILE}...\n"
		    ./${DUMMYSCRIPT} >> $SIMOUT 2>&1
		    [ $? -ne 0 ] && {
		      TOTALERRORS=$(($TOTALERRORS + 1))
		    }
		    rm ${DUMMYSCRIPT}
		  fi
		done
		echo -ne "All done for ${TESTNAME}\n\n"
		HTML_PCC=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-pcc.htm
		initialize_html $HTML_PCC "${TESTNAME} accsim simulation precompilation results"
		format_html_output $COMPOUT $HTML_PCC
		finalize_html $HTML_PCC ""
		rm $COMPOUT
						
		HTML_RUN=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-run.htm
		initialize_html $HTML_RUN "${TESTNAME} accsim simulator output"
						
		# Process output here!
		SIMSPEED=`sed -n -e '/Simulation speed/a\<br\>' -e '/Simulation speed/{s/.*Simulation speed: //;s/ /\&nbsp;/g;p}' <$SIMOUT 2>/dev/null` 
		SIMSPEED=`sed -e 's/<br>$//' <<<$SIMSPEED`
		NUMINSTRS=`sed -n -e '/Number of instructions executed/a\<br\>' -e '/Number of instructions executed/{s/.*Number of instructions executed: //;s/ /\&nbsp;/g;p}' <$SIMOUT 2>/dev/null`
		NUMINSTRS=`sed -e 's/<br>$//' <<<$NUMINSTRS`
		format_html_output $SIMOUT $HTML_RUN
		finalize_html $HTML_RUN ""
		rm $SIMOUT 2>/dev/null
		HTML_DIFF=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-diff.htm
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
		echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-pcc.htm\">simulator compilation</a>" >> $HTMLMAIN
		echo -ne ", <a href=\"${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-run.htm\">simulator output</a>" >> $HTMLMAIN
		echo -ne ", <a href=\"${HTMLPREFIX}-${ARCH}-${TESTNAME}-accsim-diff.htm\">diff output</a>)</td>" >> $HTMLMAIN
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
  HTMLSTATS=${LOGROOT}/${HTMLPREFIX}-${ARCH}-accsim-mibench-stats.htm
  initialize_html $HTMLSTATS "Mibench for ${ARCH} instruction usage data"
  cd ${STATSROOT}  
  python collect_stats.py ${ARCH} &> /dev/null
  format_html_output total.${ARCH}.stats $HTMLSTATS
  finalize_html $HTMLSTATS ""
  echo -ne "<p><B>Statistical information about instructions usage per category is available <a href=\"${HTMLPREFIX}-${ARCH}-accsim-mibench-stats.htm\">here</a>.</B></p>\n" >> $HTMLMAIN
}

### Creating HTML's headers and general structure ###
HTMLMAIN=${LOGROOT}/${HTMLPREFIX}-${ARCH}-accsim.htm
echo -ne "<html> <head> <title> ${ARCH} Compiled Simulator - Mibench Results </title> </head><body>" > $HTMLMAIN
echo -ne "<h1>${ARCH} Compiled Simulator - Mibench Results</h1>" >> $HTMLMAIN
DATE=`date '+%a %D %r'`
echo -ne "<p>Produced by NightlyTester @ ${DATE}</p>"   >> $HTMLMAIN
echo -ne "<p>Parameters used when launching accsim: ${ACCSIMFLAGS}</p>"   >> $HTMLMAIN
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLMAIN

#echo -ne "<tr><th>Program</th><th>Program</th><th>Small</th><th></th><th></th><th>Large</th><th></th><th></th></tr>\n" >> $HTMLMAIN
echo -ne "<tr><th>Name</th><th>Compilation</th><th>Simulation (small)</th><th>Speed</th><th># Instrs.</th><th>Simulation (large)</th><th>Speed</th><th># Instrs.</th></tr>\n" >> $HTMLMAIN

echo -ne "<tr><td>Automotive</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN
#basicmath
[ "$BASICMATH" != "no" ] && {
	echo -ne "\nCurrently testing: BASICMATH\n"
	echo -ne "<tr><td>basicmath</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/automotive/basicmath
	compile_prog "basicmath"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/basicmath" "output_small.txt" "yes" "$RUNSMALL" "basicmath-small" "${BENCHROOT}/automotive/basicmath" "basicmath_small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/basicmath" "output_large.txt" "yes" "$RUNLARGE" "basicmath-large" "${BENCHROOT}/automotive/basicmath" "basicmath_large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#bitcount
[ "$BITCOUNT" != "no" ] && {
	echo -ne "\nCurrently testing: BITCOUNT\n"
	echo -ne "<tr><td>bitcount</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/automotive/bitcount
	compile_prog "bitcount"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/bitcount" "output_small.txt" "yes" "$RUNSMALL" "bitcount-small" "${BENCHROOT}/automotive/bitcount" "bitcnts"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/bitcount" "output_large.txt" "yes" "$RUNLARGE" "bitcount-large" "${BENCHROOT}/automotive/bitcount" "bitcnts"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


#qsort  -- problemas: lembrar de corrigir o fonte, que causa falha de segmentacao se MAXARRAY for grande! (sugestao corrigir para MAXARRAY=10000)
[ "$QUICKSORT" != "no" ] && {
	echo -ne "\nCurrently testing: QSORT\n"
	echo -ne "<tr><td>qsort</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/automotive/qsort
	compile_prog "quicksort"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/qsort" "output_small.txt" "yes" "$RUNSMALL" "qsort-small" "${BENCHROOT}/automotive/qsort" "qsort_small"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/qsort" "output_large.txt" "yes" "$RUNLARGE" "qsort-large" "${BENCHROOT}/automotive/qsort" "qsort_large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}


#susan
[ "$SUSAN" != "no" ] && {
	echo -ne "\nCurrently testing: SUSAN\n"
	echo -ne "<tr><td>susan</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/automotive/susan
	compile_prog "susan"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/automotive/susan" "output_small.smoothing.pgm output_small.edges.pgm output_small.corners.pgm" "no" "$RUNSMALL" "susan-small" "${BENCHROOT}/automotive/susan" "susan"
	run_test "runme_large.sh" "${GOLDENROOT}/automotive/susan" "output_large.smoothing.pgm output_large.edges.pgm output_large.corners.pgm" "no" "$RUNLARGE" "susan-large" "${BENCHROOT}/automotive/susan" "susan"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- telecomm ---
echo -ne "<tr><td>Telecomm</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#adpcm - problema: Remover string produzida pelo "SystemC" no início do arquivo binário de saída - resolvido
[ "$ADPCM" != "no" ] && {
	echo -ne "\nCurrently testing: ADPCM\n"
	echo -ne "<tr><td>adpcm</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/telecomm/adpcm/src
	compile_prog "adpcm"
	cd ..
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/adpcm" "output_small.adpcm output_small.pcm" "yes" "$RUNSMALL" "adpcm-small" "${BENCHROOT}/telecomm/adpcm" "bin/rawcaudio bin/rawdaudio"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/adpcm" "output_large.adpcm output_large.pcm" "yes" "$RUNLARGE" "adpcm-large" "${BENCHROOT}/telecomm/adpcm" "bin/rawcaudio bin/rawdaudio"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
#CRC32
[ "$CRC" != "no" ] && {
	echo -ne "\nCurrently testing: CRC32\n"
	echo -ne "<tr><td>crc32</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/telecomm/CRC32
	compile_prog "crc"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/CRC32" "output_small.txt" "yes" "$RUNSMALL" "crc32-small" "${BENCHROOT}/telecomm/CRC32" "crc"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/CRC32" "output_large.txt" "yes" "$RUNLARGE" "crc32-large" "${BENCHROOT}/telecomm/CRC32" "crc"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
#FFT
[ "$FFT" != "no" ] && {
	echo -ne "\nCurrently testing: FFT\n"
	echo -ne "<tr><td>fft</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/telecomm/FFT
	compile_prog "fft"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/FFT" "output_small.txt output_small.inv.txt" "yes" "$RUNSMALL" "fft-small" "${BENCHROOT}/telecomm/FFT" "fft"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/FFT" "output_large.txt output_large.inv.txt" "yes" "$RUNLARGE" "fft-large" "${BENCHROOT}/telecomm/FFT" "fft"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#gsm
[ "$GSM" != "no" ] && {
	echo -ne "\nCurrently testing: GSM\n"
	echo -ne "<tr><td>gsm</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/telecomm/gsm
	compile_prog "gsm"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/telecomm/gsm" "output_small.encode.gsm output_small.decode.run" "yes" "$RUNSMALL" "gsm-small" "${BENCHROOT}/telecomm/gsm" "bin/toast bin/untoast"
	run_test "runme_large.sh" "${GOLDENROOT}/telecomm/gsm" "output_large.encode.gsm output_large.decode.run" "yes" "$RUNLARGE" "gsm-large" "${BENCHROOT}/telecomm/gsm" "bin/toast bin/untoast"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- network ---
echo -ne "<tr><td>Network</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#dijkstra
[ "$DIJKSTRA" != "no" ] && {
	echo -ne "\nCurrently testing: DIJKSTRA\n"
	echo -ne "<tr><td>dijkstra</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/network/dijkstra
	compile_prog "dijkstra"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/network/dijkstra" "output_small.dat" "yes" "$RUNSMALL" "dijkstra-small" "${BENCHROOT}/network/dijkstra" "dijkstra_small"
	run_test "runme_large.sh" "${GOLDENROOT}/network/dijkstra" "output_large.dat" "yes" "$RUNLARGE" "dijkstra-large" "${BENCHROOT}/network/dijkstra" "dijkstra_large"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#patricia
[ "$PATRICIA" != "no" ] && {
	echo -ne "\nCurrently testing: PATRICIA\n"
	echo -ne "<tr><td>patricia</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/network/patricia
	compile_prog "patricia"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/network/patricia" "output_small.txt" "yes" "$RUNSMALL" "patricia-small" "${BENCHROOT}/network/patricia" "patricia"
	run_test "runme_large.sh" "${GOLDENROOT}/network/patricia" "output_large.txt" "yes" "$RUNLARGE" "patricia-large" "${BENCHROOT}/network/patricia" "patricia"
	echo -ne "</tr>\n" >> $HTMLMAIN
}
# --- security ---
echo -ne "<tr><td>Security</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#rijndael
[ "$RIJNDAEL" != "no" ] && {
	echo -ne "\nCurrently testing: RIJNDAEL\n"
	echo -ne "<tr><td>rijndael</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/security/rijndael
	compile_prog "rijndael"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/security/rijndael" "output_small.enc output_small.dec" "no" "$RUNSMALL" "rijndael-small" "${BENCHROOT}/security/rijndael" "rijndael"
	run_test "runme_large.sh" "${GOLDENROOT}/security/rijndael" "output_large.enc output_large.dec" "no" "$RUNLARGE" "rijndael-large" "${BENCHROOT}/security/rijndael" "rijndael"
	echo -ne "</tr>\n" >> $HTMLMAIN
}	

#sha
[ "$SHA" != "no" ] && {
	echo -ne "\nCurrently testing: SHA\n"
	echo -ne "<tr><td>sha</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/security/sha
	compile_prog "sha"
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/security/sha" "output_small.txt" "yes" "$RUNSMALL" "sha-small" "${BENCHROOT}/security/sha" "sha"
	run_test "runme_large.sh" "${GOLDENROOT}/security/sha" "output_large.txt" "yes" "$RUNLARGE" "sha-large" "${BENCHROOT}/security/sha" "sha"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

# --- consumer ---
echo -ne "<tr><td>Consumer</td><td></td><td></td><td></td><td></td><td></td><td></td><td></td></tr>\n" >> $HTMLMAIN

#jpeg
[ "$JPEG" != "no" ] && {
	echo -ne "\nCurrently testing: JPEG\n"
	echo -ne "<tr><td>jpeg</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/consumer/jpeg/jpeg-6a
	compile_prog "jpeg"
	cd ..
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/consumer/jpeg" "output_small_encode.jpeg output_small_decode.ppm" "no" "$RUNSMALL" "jpeg-small" "${BENCHROOT}/consumer/jpeg" "jpeg-6a/cjpeg jpeg-6a/djpeg"
	run_test "runme_large.sh" "${GOLDENROOT}/consumer/jpeg" "output_large_encode.jpeg output_large_decode.ppm" "no" "$RUNLARGE" "jpeg-large" "${BENCHROOT}/consumer/jpeg" "jpeg-6a/cjpeg jpeg-6a/djpeg"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

#lame
[ "$LAME" != "no" ] && {
	echo -ne "\nCurrently testing: LAME\n"
	echo -ne "<tr><td>lame</td>" >> $HTMLMAIN
	cd ${BENCHROOT}/consumer/lame/lame3.70
	compile_prog "lame"
	cd ..
	chmod u+x *.sh
	run_test "runme_small.sh" "${GOLDENROOT}/consumer/lame" "output_small.mp3" "no" "$RUNSMALL" "lame-small" "${BENCHROOT}/consumer/lame" "lame3.70/lame"
	run_test "runme_large.sh" "${GOLDENROOT}/consumer/lame" "output_large.mp3" "no" "$RUNLARGE" "lame-large" "${BENCHROOT}/consumer/lame" "lame3.70/lame"
	echo -ne "</tr>\n" >> $HTMLMAIN
}

echo -ne "</table>\n" >> $HTMLMAIN

# Collect statistical information 
if [ "$COLLECT_STATS" != "no" ]; then
  build_stats
fi

finalize_html $HTMLMAIN ""