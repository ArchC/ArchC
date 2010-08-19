# run_teste.sh
# Testa apenas um programa do ac stone. gerando uma saida do gdb e uma saida dos stats.Recebe como parametros o numero do programa e a arquitetura.Deve estar 
# localizodo na mesma pasta dos binarios dos programas.
#
# Roblêdo Camilo de Alencar
#
# 10/06/2010
#

# Param1 is the input raw text to format
# Param2 is the HTML file name
format_html_output() {
	echo -ne "<table><tr><td><font face=\"Courier\">" >>$2
	sed -e 'a\<br\>' -e 's/ /\&nbsp;/g' <$1 1>>$2 2>/dev/null
	echo -ne "</font></td></tr></table>"  >>$2
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

# Commented out variables are now defined by the caller script (nightlytester.sh)
#coloque aqui o caminho para o simulador
#SIMULATOR=/home/ec2007/ra074990/mips1-v0.7.8/mips1.x
#GDB=/home/ec2007/ra074990/mips1-v0.7.8/acstone/gdbout
if test ! $# -eq 2 || test "$1" == "--help" 
then
    echo "This program run the GNU Debugger for one program" 1>&2
    echo "Use: $0 PROGRAM ARCH" 1>&2
    exit 1
fi

ARCH=$2
PROGRAM=$1
BINARY=`ls ${PROGRAM}.*.${ARCH}`
NLCFG=`cat gdb/firstcommands.gdb | wc -l`
NAME=`echo ${BINARY/.${ARCH}/}`
NL=`cat gdb/${NAME}.gdb | wc -l`
NLSHOW=`expr ${NL} - 2`
rm -f ${NAME}.cmd
cat gdb/firstcommands.gdb > ${NAME}.cmd
tail -n ${NLSHOW} gdb/${NAME}.gdb >> ${NAME}.cmd

echo "<tr><td>${NAME}</td>" >> $HTMLACSTONE
echo "running test ${PROGRAM}"
${SIMULATOR} --load=${BINARY} --gdb 2> ${NAME}.${ARCH}.stats& 
sleep 3
$GDBEXEC ${BINARY} --command=${NAME}.cmd | cut -s -f 2 -d '$' | cut -f 2 -d '=' > ${NAME}.${ARCH}.out

HTML_DIFF=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${NAME}-acstone-diff.htm
HTML_SIMOUT=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${NAME}-acstone-simout.htm
HTML_GDBOUT=${LOGROOT}/${HTMLPREFIX}-${ARCH}-${NAME}-acstone-gdbout.htm
initialize_html $HTML_DIFF "Acstone ${NAME} - ${ARCH} - output compared with golden model"
initialize_html $HTML_SIMOUT "Acstone ${NAME} - ${ARCH} - simulator output"
initialize_html $HTML_GDBOUT "Acstone ${NAME} - ${ARCH} - GDB output (data prints)"
TEMPFL=${random}.out
diff ${BINARY}.out data/${NAME}.data &> $TEMPFL
RETCODE=$?
format_html_output $TEMPFL $HTML_DIFF
format_html_output ${BINARY}.stats $HTML_SIMOUT
format_html_output ${BINARY}.out $HTML_GDBOUT
finalize_html $HTML_DIFF ""
finalize_html $HTML_SIMOUT ""
finalize_html $HTML_GDBOUT ""
rm $TEMPFL


if [ $RETCODE -ne 0 ]; then
  echo -ne "<td><b><font color=\"crimson\"> Failed </font></b>" >> $HTMLACSTONE
else
  echo -ne "<td><b><font color=\"green\"> OK </font></b>" >> $HTMLACSTONE
fi
echo -ne "(<a href=\"${HTMLPREFIX}-${ARCH}-${NAME}-acstone-diff.htm\">diff output</a>, \n" >> $HTMLACSTONE
echo -ne  "<a href=\"${HTMLPREFIX}-${ARCH}-${NAME}-acstone-simout.htm\">simulator output</a>, \n" >> $HTMLACSTONE
echo -ne  "<a href=\"${HTMLPREFIX}-${ARCH}-${NAME}-acstone-gdbout.htm\">GDB output</a>" >> $HTMLACSTONE
echo -ne ")</td>\n" >> $HTMLACSTONE

sleep 1