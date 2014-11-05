# run_all.sh
# Executa testes para todo o pacote ac stone, validando a saida do gdb e produzindo os stats para todos.Recebe como parametro a arquitetura para a qual devem 
# ser feitos os testes.Deve estar na mesma pasta onde estão localizados os binarios dos programas, junto com run_teste.sh e stats.py.
#
# Roblêdo Camilo de Alencar
#
# 10/06/2010
#
ARCH=$1

# Commented out variables are defined by the caller script (nightlytester.sh)
#TESTER=run_teste.sh
#cooloque aqui o caminho para o somador stats.py, que deve estar na pasta onde estão os arquivos stats
#SOMADOR=/home/ec2007/ra074990/mips1-v0.7.8/acstone/stats/stats.py
#LOGROOT=BLABLA
#HTMLPREFIX=BLABLA
HTMLACSTONE=${LOGROOT}/${HTMLPREFIX}-${ARCH}-acstone.htm

export HTMLACSTONE
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
# Param1 is the HTML file name
# Param2 is the "title" string
initialize_html() {
	echo -ne "<html> <head> <title> ${2} </title> </head><body>" > $1
	echo -ne "<h1>${2}</h1>" >> $1
}
# This function will sumarize statistical information about all ran programs using the collect_stats.py script.
build_stats() {
  HTMLSTATS=${LOGROOT}/${HTMLPREFIX}-${ARCH}-acstone-stats.htm
  initialize_html $HTMLSTATS "Acstone for ${ARCH} instruction usage data"
  #cd ${STATSROOT}  
  python collect_stats.py ${ARCH} &> /dev/null
  format_html_output total.${ARCH}.stats $HTMLSTATS
  finalize_html $HTMLSTATS ""
  echo -ne "<p><B>Statistical information about instructions usage per category is available <a href=\"${HTMLPREFIX}-${ARCH}-acstone-stats.htm\">here</a>.</B></p>\n" >> $HTMLACSTONE
}

if test ! $# -eq 1 || test "$1" == "--help" 
then
    echo "This program run the GNU Debugger for each program" 1>&2
    echo "Use: $0 ARCH" 1>&2
    exit 1
fi

# HTML LOG
echo -ne "<html> <head> <title> ${ARCH} Simulator - Acstone Results </title> </head><body>" > $HTMLACSTONE
echo -ne "<h1>${ARCH} Simulator - Acstone Results</h1>" >> $HTMLACSTONE
DATE=`date '+%a %D %r'`
echo -ne "<p>Produced by NightlyTester @ ${DATE}</p>"   >> $HTMLACSTONE
echo -ne "<p><table border=\"1\" cellspacing=\"1\" cellpadding=\"5\">" >> $HTMLACSTONE

for I in `ls *.${ARCH}`
  do
  
  ${TESTER} ${I:0:3} ${ARCH}
done
#python ${SOMADOR} ${ARCH}


echo -ne "</table>\n" >> $HTMLACSTONE

# Collect statistical information 
build_stats

finalize_html $HTMLACSTONE ""