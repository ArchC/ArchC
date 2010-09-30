# -*- coding: utf-8 -*-
# stats.py
# É executado pelo run_all.sh.Gera um arquivo com o total de instruções executadas pela plataforma em todos os testes.Este script deve estar na mesma pasta onde estão os arquivos stats. 
#
# Roblêdo Camilo de Alencar
#
# 26/06/2010
#
import os
import fnmatch
import string
import re
import sys
import getopt
#especifique o caminho para os arquivos stats abaixo
stats = os.getcwd()
filePattern = fnmatch.translate ('*.'+sys.argv[1]+'.stats' )
cabecalho = re.compile('\[ArchC 2.1] Printing statistics from instruction (?P<inst>\w*):')
count = re.compile(' *COUNT : (?P<num>\d*)')
total = dict()
coverage = 0
for filename in os.listdir (stats):
	if re.match ( filePattern, filename ):
		filehandle = open(stats+'/'+filename, 'r')
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
filehandle = open(stats+'/'+'total.'+sys.argv[1]+'.stats', 'w')
instotal = sum([i for i in total.values()])
for key in total.keys():
	if total[key] != 0:
		coverage = coverage + 1 
	filehandle.write(key+' : '+str(total[key])+'  '+str(total[key]*100/instotal)+'%\n')
filehandle.write('coverage : '+str(coverage*100/len(total))+'%\n')
filehandle.close
