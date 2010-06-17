# stats.py
# É executado pelo run_all.sh.Gera um arquivo com o total de instruções executadas pela plataforma em todos os testes.Este script deve estar na mesma pasta onde estão os arquivos stats. 
#
# Roblêdo Camilo de Alencar
#
# 10/06/2010
#
import os
import fnmatch
import string
import re
import sys
import getopt

filePattern = fnmatch.translate ('*.'+sys.argv[1]+'.stats' )
cabecalho = re.compile('\[ArchC 2.0] Printing statistics from instruction (?P<inst>\w*):')
count = re.compile(' *COUNT : (?P<num>\d*)')
total = dict()
for filename in os.listdir (os.getcwd()):
	if re.match ( filePattern, filename ):
		filehandle = open(filename, 'r')
		lines = filehandle.readlines()
		for line in lines:
			tmp = cabecalho.match(line)
			if tmp is not None:
				instrucao = tmp.group('inst')
				if instrucao not in total.keys():
					total[instrucao] = 0
			tmp = count.match(line)
			if tmp is not None:
				total[instrucao] = total[instrucao] + int(tmp.group('num'))
		filehandle.close()
filehandle = open('total.'+sys.argv[1]+'.stats', 'w')
for key in total.keys():
	filehandle.write(key+' : '+str(total[key])+'\n')
filehandle.close
