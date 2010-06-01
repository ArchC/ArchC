# -*- coding: utf-8 -*-
from sys import argv
from os import environ

environ['path'] = ""
environ['doxypath'] = ""

def help():
    print "Help here"

try:
    if len(argv) < 2:
	raise IOError
    else:
	if argv[1] == '-h' or argv[1] == '--help':
	    help()
	elif argv[1].startswith("-"):
	    raise IOError
	else:
	    for i in range(2,len(argv)):
		if argv[i].startswith('-target='):
		    path = argv[i].replace('-target=','')
		    environ['path'] = path.replace('"','')
		    print environ['path']
		elif argv[i].startswith('-doxygen='):
		    path = argv[i].replace('-doxygen=','')
		    environ['doxypath'] = path.replace('"','')
		    print environ['doxypath']
		elif argv[i] == '-h' or argv[i] == '--help':
		    print "Help here"

	    from acParser import acParser
	    parser = acParser()
	    parser.ParseFile(argv[1])
	    parser.docFile()
except IOError:
    print "Error: Input file needed to execute parser, use 'python acdoc.py -h' for usage instructions."