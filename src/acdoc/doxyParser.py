# -*- coding: utf-8 -*-
def getAttr(s,subs):
    subs += '="'
    a = s.find(subs) + len(subs)
    b = s.find('"',a)
    return s[a:b]

def getAnchors(filename):
    if filename == "":
	return { }
    f = open(filename,'r')
    lines = f.readlines()
    anchors = []
    links = {}
    for line in lines:
	if line.startswith('<a class="anchor"'):
	    anchors.append(line[0:-1])

    for item in anchors:
	ref = '#' + getAttr(item,"id")
	arg = getAttr(item,"args")
	arg = arg.replace('(','')
	arg = arg.replace(')','')
	if arg:
	    links[arg] = ref

    return links


#getAnchors("mips1-isa_8cpp.html")
