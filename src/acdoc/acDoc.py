# -*- coding: utf-8 -*-
"""	acDoc.py

	This file contains the main class of the ArchC HTML Document Generator, aka, acdoc.
	We use basically the same idea implemented in Doxygen: given the information generated
	at the parser phase of the compiler, and the Abstract Syntax Tree of the ArchC language,
	it's possible to create a properly structured document with the elements provided.
	
	Every element of the AST extends and abstract class called Entry. It contains only three
	methods: __init__, getContentWithLink (link generator), generateHtml (HTML code generator).
	
	There are two main functions given to links:
	1. Link structures and elements within the code
	2. Link types and specific reserved language words with the Official Language Reference
	
	The main class of this module, acdoc, receives 2 sets of information from the parser: first
	what we call here "code-info", which means information extracted from non-comment parts of
	the code and also "comment-info", which is extracted from the '/**' type of comments. Then
	there is the 'makeAST' method, that will create a list of Entries to be processed by the
	'generateHtml' method.
"""

from os import environ
import doxyParser

# This dictionary contains all ArchC reserved words, now with temporary links to the ArchC Homepage
ac_links = {'AC_ISA' : 'http://www.archc.org/', 'ISA_CTOR' : 'http://www.archc.org/',
	'set_decoder': 'http://www.archc.org/', 'set_asm' : 'http://www.archc.org/',
	'set_cycles' : 'http://www.archc.org/', 'cycle_range' : 'http://www.archc.org/',
	'ac_format' : 'http://www.archc.org/', 'ac_instr' : 'http://www.archc.org/',
	'ac_group' : 'http://www.archc.org/', 'ac_helper' : 'http://www.archc.org/',
	'ac_asm_map' : 'http://www.archc.org/', 'pseudo_instr' : 'http://www.archc.org/',
	'is_jump' : 'http://www.archc.org/', 'is_branch' : 'http://www.archc.org/',
	'cond' : 'http://www.archc.org/', 'delay' : 'http://www.archc.org/',
	'delay_cond' : 'http://www.archc.org/', 'behavior' : 'http://www.archc.org/',
	'AC_ARCH' : 'http://www.archc.org/', 'ARCH_CTOR' : 'http://www.archc.org/',
	'ac_tlm_port' : 'http://www.archc.org/', 'ac_tlm_intr_port' : 'http://www.archc.org/',
	'ac_cache' : 'http://www.archc.org/', 'ac_icache' : 'http://www.archc.org/',
	'ac_dcache' : 'http://www.archc.org/', 'ac_mem' : 'http://www.archc.org/',
	'ac_isa': 'http://www.archc.org/',
	'ac_reg': 'http://www.archc.org/',  'ac_regbank' : 'http://www.archc.org/', 
	'ac_format' : 'http://www.archc.org/', 'ac_wordsize' : 'http://www.archc.org/',
	'ac_fetchsize' : 'http://www.archc.org/', 'ac_stage' : 'http://www.archc.org/',
	'ac_pipe' : 'http://www.archc.org/', 'set_endian' : 'http://www.archc.org/',
	'bindTo' : 'http://www.archc.org/', 'bindsTo' : 'http://www.archc.org/'
	}

doxy_path = environ['doxypath']
doxy_links = doxyParser.getAnchors(doxy_path)

source_link = "[see source code]"

class Entry:
	""" Abstract type for the Abstract Syntax Tree.
	"""
	def __init__(self):
		self.name = ""
		
	def getLink(self,content):
		try:
			link = ac_links[content]
		except(Exception):
			link = None
		if  link != None:
			return "<a href='"+link+"'>"+content+"</a>" 
		else:
			return content
			
	def generateHtml(self):
		return ""

	def doxyLink(self):
		if doxy_path == "":
		    return self.name
		else:
		    try:
			anchor = doxy_path + doxy_links[self.name]
			link = "<a href='"+anchor+"'>"+self.name+"</a>"
			return link
		    except KeyError:
			return self.name
			
	def sourceLink(self,expr,src):
	    f = open(src,'r')
	    lines = f.readlines()
	    for i in range(len(lines)):
		c = 0
		for e in expr:
		    if lines[i].find(e) == -1:
			break;
		    else:
			c += 1
		if c == len(expr):	
		    name = src.split('.')[0] + '_source.html'
		    link = "&nbsp&nbsp&nbsp&nbsp<a href="+name+"#l"+str(i+1)+">"+source_link+"</a>"
		    return link

class ac_reg(Entry):
	""" This class represents a register entry in the AST
	"""
	def __init__(self,name,value):
		self.classname = 'ac_reg'
		self.name = name
		self.value = value
		
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		if self.value != None:
		    value = "Value: "+str(self.value)+"<br/>\n"
		else:
		    value = ""
		html =  "<div class='ac_reg'>Register (" + self.getLink('ac_reg') + "):<div class='desc'>" + name + value + "</div></div>\n"
		return html

class ac_regbank(Entry):
	""" This class represents a register bank in the ArchC Language
	"""
	def __init__(self,name,value):
		self.classname = 'ac_regbank'
		self.name = name
		self.value,self.param = value
	
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		value = "Size: "+str(self.value)+"<br/>\n"
		if self.param != None:
		    param = "Parameter: "+str(self.param)+"<br/>\n"
		else:
		    param = ""
		html =  "<div class='ac_regbank'>Register bank (" + self.getLink('ac_regbank') + "): <div class='desc'>" + name + value + param + "</div></div>\n"
		return html
		
class ac_mem(Entry):
	""" This class represents a memory in the ArchC Language
	"""
	def __init__(self,name,value):
		self.classname = 'ac_mem'
		self.name = name
		self.value = value
		
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		value = "Size: "+str(self.value)+"<br/>\n"
		html =  "<div class='ac_mem'>Memory (" + self.getLink('ac_mem') + "):<div class='desc'>" + name + value + "</div></div>\n"
		return html

class ac_cache(Entry):
	""" This class represents a cache in the ArchC Language
	"""
	def __init__(self,name,value):
		self.classname = 'ac_cache'
		self.name = name
		self.value = value
		
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		value = "Size: "+str(self.value)+"<br/>\n"
		html =  "<div class='ac_icache'>Cache (" + self.getLink('ac_cache') + "): <div class='desc'>" + name + value + "</div></div>\n"
		return html

class ac_icache(Entry):
	""" This class represents a cache in the ArchC Language
	"""
	def __init__(self,name,value):
		self.classname = 'ac_icache'
		self.name = name
		self.value = value
		
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		value = "Size: "+str(self.value)+"<br/>\n"
		html =  "<div class='ac_icache'>Instructions Cache (" + self.getLink('ac_icache') + "): <div class='desc'>" + name + value + "</div></div>\n"
		return html

class ac_dcache(Entry):
	""" This class represents a cache in the ArchC Language
	"""
	def __init__(self,name,value):
		self.classname = 'ac_dcache'
		self.name = name
		self.value = value
		
	def generateHtml(self):
		name = "Name: "+self.name + self.sourceLink([self.classname,self.name],environ['arch']+".ac") + "<br/>\n"
		value = "Size: "+str(self.value)+"<br/>\n"
		html =  "<div class='ac_dcache'>Data Cache (" + self.getLink('ac_dcache') + "):<div class='desc'>" + name + value + "</div></div>\n"
		return html


		
class ac_wordsize(Entry):
	""" This class represents the wordsize of the given architecture
	"""
	def __init__(self,size):
		self.classname = 'ac_wordsize'
		self.value = size
		
	def generateHtml(self):
		value = "<div class='desc'>Size: "+str(self.value)+ self.sourceLink([self.classname],environ['arch']+".ac") +"</div>\n"
		html =  "<div class='ac_wordsize'>Word size (" + self.getLink('ac_wordsize') + "):" + value + "</div>\n"
		return html
		
class ac_fetchsize(Entry):
	""" This class represents the wordsize of the given architecture
	"""
	def __init__(self,size):
		self.classname = 'ac_fetchsize'
		self.value = size
		
	def generateHtml(self):
		value = "<div class='desc'>Value: "+str(self.value)+ self.sourceLink([self.classname],environ['arch']+".ac") + "</div>\n"
		html =  "<div class='ac_fetchsize'>Fetch size (" + self.getLink('ac_fetchsize') + "):" + value + "</div>\n"
		return html
		
class arch_ctor(Entry):
	""" Entry for architecture constructor
	"""
	def __init__(self,isaName,endian,binds = []):
		uppercase = {'big': 'Big', 'little': 'Little'}
		self.isaName = isaName
		self.endian = uppercase[endian]
		self.bind = binds
		
	def generateHtml(self):
		isaName = "ISA Name: "+self.isaName + self.sourceLink(['ac_isa'],environ['arch']+".ac") + "<br/>\n"
		endian = self.endian + " Endian"+self.sourceLink(['set_endian'],environ['arch']+".ac")+"<br/>\n"
		if self.bind != []:
		    bind = "Binds to: "+str(self.bind)+self.sourceLink(['bind','To'],environ['arch']+".ac")+"<br/>\n"
		else:
		    bind = ""
		html =  "<div class='arch_ctor'>Architecture Constructor (" + self.getLink('ARCH_CTOR') + "):<div class='desc'>" + isaName + endian + bind + "</div></div>\n"
		return html
		
class ac_stage(Entry):
	""" This class represents a stage in a processor.
	"""
	def __init__(self,name):
		self.classname = 'ac_stage'
		self.name = name
		
	def generateHtml(self):
		name = "<div class='desc'>Name: "+self.name+self.sourceLink([self.classname,self.name],environ['arch']+".ac")+"</div>\n"
		html =  "<div class='ac_stage'>Stage (" + self.getLink('ac_stage') + "):" + name + "</div>\n"
		return html
		
class ac_pipe(Entry):
	""" This class represents the architecture pipeline.
	"""
	def __init__(self,name,attr):
		self.classname = 'ac_pipe'
		self.name = name
		self.attr = attr
		
	def generateHtml(self):
		name = "Name: "+self.name+self.sourceLink([self.classname,self.name],environ['arch']+".ac")+"<br/>\n"
		attr = "Attribute: "+str(self.attr)+"<br/>\n"
		html =  "<div class='ac_pipe'>Pipe (" + self.getLink('ac_pipe') + "): <div class='desc'>" + name + attr + "</div></div>\n"
		return html
		
class ac_arch(Entry):
	""" This is a more specific entry, which generates code for printing the architecture name.
	"""
	def __init__(self,archName):
		self.comment = None
		self.name = archName
		self.mems = []
		self.regs = []
		self.regbanks = []
		self.caches = []
		self.icaches = []
		self.dcaches = []
		self.wordsize = None
		self.fetchsize = None
		self.constructor = None
		self.pipelines = []
		self.stages = []
	
	def generateHtml(self):
		
		name = "<div class='arch_name'>Name: " + self.name + "</div>\n"
		
		memsHtml = ""
		for mem in self.mems:
			memsHtml = memsHtml + mem.generateHtml()
		
		regsHtml = ""
		for reg in self.regs:
			regsHtml = regsHtml +  reg.generateHtml()
		
		regbanksHtml = ""
		for regbank in self.regbanks:
			regbanksHtml = regbanksHtml + regbank.generateHtml()
		
		cachesHtml = ""
		for cache in self.caches:
			cachesHtml = cachesHtml + cache.generateHtml()
		for icache in self.icaches:
			cachesHtml = cachesHtml + icache.generateHtml()
		for dcache in self.dcaches:
			cachesHtml = cachesHtml + dcache.generateHtml()
			
		pipelinesHtml = ""
		for pipeline in self.pipelines:
			pipelinesHtml = pipelinesHtml + pipeline.generateHtml()
			
		stagesHtml = ""
		for stage in self.stages:
			stagesHtml = stagesHtml + stage.generateHtml()
		
		wordHtml = ""
		if self.wordsize != None:
			wordHtml = self.wordsize.generateHtml()
		
		fetchHtml = ""
		if self.fetchsize != None:
			fetchHtml = self.fetchsize.generateHtml()
			
		constructorHtml = ""
		if self.constructor != None:
			constructorHtml = self.constructor.generateHtml()
		
		commentHtml = ""
		if self.comment != None:
			for k in self.comment.keys():
				commentHtml = commentHtml + k + ":<br/>" + self.comment[k] + "<br/><br/>"
			commentHtml = "<div class='comment'>" + commentHtml + "</div>"
		
		html = ""
		html +=  "<div class='ac_arch'>Architecture (" + self.getLink('AC_ARCH') + "):" + name + commentHtml + memsHtml + regsHtml + regbanksHtml + cachesHtml
		html += wordHtml + fetchHtml + constructorHtml + pipelinesHtml + stagesHtml + "</div>\n"

		return html

class ac_helper(Entry):
    
    def __init__(self,content):
		self.content = content
		self.content = self.content.replace('\n','<br/>')
		

    def generateHtml(self):
		content = "<div class='desc'>"+str(self.content)+"</div>\n"
		html =  "<div class='ac_helper'>Helper (" + self.getLink('ac_helper') + "):" + content + "</div>\n"
		return html

class set_decoder(Entry):

    def __init__(self,value):
	self.values = ""
	for i in value:
	    x,y = i
	    self.values += x + "=" + str(y) + "; "
	
    def generateHtml(self):
	html = "Decoder: "+ self.values + "<br/>\n"
	return html
	
class set_asm(Entry):

    def __init__(self,value):
	self.value = str(value)
	
    def generateHtml(self):
	html = "ASM: "+ self.value +"<br/>\n"
	return html

class is_jump(Entry):
    
    def __init__(self,value):
	self.value = value[0]
	self.value = self.value[0:-2]

    def generateHtml(self):
	html = "Is Jump: "+ self.value +"<br/>\n"
	return html

class is_branch(Entry):
    
    def __init__(self,value):
	    self.value = value[0]
	    self.value = self.value[0:-2]

    def generateHtml(self):
	    html = "Is Branch: "+ self.value +"<br/>\n"
	    return html
	
class delay(Entry):
    
    def __init__(self,value):
	self.value = value[0]
	self.value = self.value[0:-2]

    def generateHtml(self):
	html = "Delay: "+ self.value +"<br/>\n"
	return html

class delay_cond(Entry):
    
    def __init__(self,value):
	self.value = value[0]
	self.value = self.value[0:-2]

    def generateHtml(self):
	html = "Delay Condition: "+ self.value +"<br/>\n"
	return html

class cond(Entry):
    
    def __init__(self,value):
	self.value = value[0]
	self.value = self.value[0:-2]

    def generateHtml(self):
	html = "Condition: "+ self.value +"<br/>\n"
	return html

class behavior(Entry):
    
    def __init__(self,value):
	self.value = value[0]
	self.value = self.value[0:-2]

    def generateHtml(self):
	html = "Behavior: "+ self.value +"<br/>\n"
	return html

class ac_instr(Entry):
    
    def __init__(self,name,itype):
		self.name = name
		self.itype = itype
		self.decoder = None
		self.isjumps = []
		self.isbranchs = []
		self.delays = []
		self.conds = []
		self.delay_conds = []
		self.asms = []
		self.behaviors = []
	
    def generateHtml(self):
		itype =  "Instruction Type: <a href=#"+self.itype+">"+self.itype+"</a><br/>\n"
		decoder = self.decoder.generateHtml()
		
		isjumps = ""
		for i in self.isjumps:
		    isjumps += i.generateHtml()
		
		isbranchs = ""
		for i in self.isbranchs:
		    isbranchs += i.generateHtml()

		delays = ""
		for i in self.delays:
		    delays += i.generateHtml()

		delay_conds = ""
		for i in self.delay_conds:
		    delay_conds += i.generateHtml()

		conds = ""
		for i in self.conds:
		    conds += i.generateHtml()

		behaviors = ""
		for i in self.behaviors:
		    behaviors += i.generateHtml()
		    
		asms = ""
		for i in self.asms:
		    asms += i.generateHtml()		    

		optlist = asms + isjumps + isbranchs + delays + conds + delay_conds + behaviors
		html =  "<div class='ac_instr'>"+self.doxyLink()+" (<a name='" + self.name + "'>" + self.getLink('ac_instr') + "):"
		html += "<div class='desc'>" + itype + decoder + optlist + "</div></a></div>\n"
		return html


class ac_format(Entry):
    
    def __init__(self,name,desc,ilist):
		self.name = name
		self.desc = desc
		self.ilist = ilist
	
    def generateHtml(self):
		desc = ""
		for i in self.desc:
			tmp = ""
			for j in i:
				tmp = tmp + " " + str(j)
			desc = desc + tmp + ";"
		desc = "Format Description: " + desc + "<br/>\n"
		
		ilist = ""
		if self.ilist != []:
			for i in self.ilist:
				ilist = ilist + " <a href='#" + i + "'>" + i + "</a>"
			ilist = "Instructions: " + ilist + "</br>\n"
		html =  "<div class='ac_format'>" + self.doxyLink() + " (<a name='" + self.name + "'>" + self.getLink('ac_format') + "):</a><div class='desc'>" + desc + ilist + "</div></div>\n"
		return html

class ac_group(Entry):

    def __init__(self,name,elem):
		self.name = name
		self.elem = elem
	
    def generateHtml(self):
		elem =  "Element: "+ self.elem +"<br/>\n"
		html =  "<div class='ac_group'>" + self.name + " (" + self.getLink('ac_group') + "):<div class='desc'>" + elem + "</div></div>\n"
		return html

class ac_asm_map(Entry):

    def __init__(self,name,maps):
		self.classname = 'ac_asm_map'
		self.name = name
		self.maps = maps
	
    def generateHtml(self):
		maps = ""
		for i in self.maps:
			tmp = ""
			for j in i:
				tmp = tmp + " " + str(j)
			maps += "Maps: "+ tmp +"<br/>\n"
		html =  "<div class='ac_asm_map'>" + self.name + " (" + self.getLink('ac_asm_map') + "):"+ self.sourceLink([self.classname,self.name],environ['arch']+"_isa.ac")
		html += "<div class='desc'>" + maps + "</div></div>\n"
		return html

class pseudo_instr(Entry):

    def __init__(self,name,body):
	self.name = name
	self.body = ""
	for i in body:
	    self.body += i + "<br/>"
	    
    def generateHtml(self):
	body = "<div class='desc'>Instructions: <br/> "+ self.body +"</div>\n"
	html = "<div class='pseudo_instr'>"+ self.name +" (" + self.getLink('pseudo_instr') + "):"+self.sourceLink([self.name],environ['arch']+"_isa.ac") + body + "</div>\n"
	return html

class ac_isa(Entry):
    """		ISA Class.
    """
	
    def __init__(self,name):
		self.name = name
		self.comment = None
		self.helper = None
		self.instrlist = []
		self.formatlist = []
		self.grouplist = []
		self.maplist = []
		self.pseudolist = []

    def generateOtherHtml(self):

		name = "<div class='isa_name'>Name: "+ self.name +"</div>\n"
		
		helper = ""
		if self.helper != None:
			helper = self.helper.generateHtml()
			
		grouplist = ""
		for i in self.grouplist:
			grouplist = grouplist + i.generateHtml()
			
		maplist = ""
		for i in self.maplist:
			maplist = maplist + i.generateHtml()
			
		pseudolist = ""
		for i in self.pseudolist:
			pseudolist += i.generateHtml() 
		
		html =  "<div class='ac_isa'>ISA (" + self.getLink('ac_isa') + "):" + name + helper + grouplist + maplist + pseudolist +"</div>\n"
		return html

    def generateIsaHtml(self):

		name = "<div class='isa_name'>Name: "+ self.name +"</div>\n"
		
		instrlist = ""
		for i in self.instrlist:
			instrlist = instrlist + i.generateHtml()
			
		formatlist = ""
		for i in self.formatlist:
			formatlist = formatlist + i.generateHtml()

		commentHtml = ""
		if self.comment != None:
			for k in self.comment.keys():
				commentHtml = commentHtml + k + ":<br/>" + self.comment[k] + "<br/><br/>"
			commentHtml = "<div class='comment'>" + commentHtml + "</div>"

		html =  "<div class='ac_isa'>ISA (" + self.getLink('ac_isa') + "):" + name + commentHtml + formatlist + instrlist +"</div>\n"
		return html

class HtmlIndex():

    def __init__(self,name,ast):
	self.name = name
	self.index = open(name+"_index.html",'w')
	self.index.write("<html>\n<head>\n")
	self.generateHtml(ast)
	self.index.write("</body>\n</html>")
	self.index.close()
	
    def generateHtml(self,ast):
	html = "<link href='style.css' rel='stylesheet' type='text/css' />\n"
	html += "<title>"+ self.name +" Documentation</title>\n</head>\n<body>\n"
	html += "<div class='definfo'>This documentation was automatically generated with AcDoc tool.</div>"
	html += "<div class='definfo'><a href="+self.name+"_isa.html>"+self.name+" Instruction Set and Formats</a></div>"
	html += "<div class='definfo'><a href="+self.name+"_other.html>"+self.name+" Other Properties</a></div>"
	html += ast.generateHtml()
	self.index.write(html)

class HtmlISA():

    def __init__(self,name,ast):
	self.name = name
	self.index = open(name+"_isa.html",'w')
	self.index.write("<html>\n<head>\n")
	self.generateHtml(ast)
	self.index.write("</body>\n</html>")
	self.index.close()
	
    def generateHtml(self,ast):
	html = "<link href='style.css' rel='stylesheet' type='text/css' />\n"
	html += "<title>"+ self.name +" ISA Documentation</title>\n</head>\n<body>\n"
	html += "<div class='definfo'>This documentation was automatically generated with AcDoc tool.</div>"
	html += "<div class='definfo'><a href="+self.name+"_index.html>"+self.name+" Architechture Description</a></div>"
	html += "<div class='definfo'><a href="+self.name+"_other.html>"+self.name+" Other Properties</a></div>"
	html += ast.generateIsaHtml()
	self.index.write(html)

class HtmlOther():

    def __init__(self,name,ast):
	self.name = name
	self.index = open(name+"_other.html",'w')
	self.index.write("<html>\n<head>\n")
	self.generateHtml(ast)
	self.index.write("</body>\n</html>")
	self.index.close()
	
    def generateHtml(self,ast):
	html = "<link href='style.css' rel='stylesheet' type='text/css' />\n"
	html += "<title>"+ self.name +" ISA Documentation</title>\n</head>\n<body>\n"
	html += "<div class='definfo'>This documentation was automatically generated with AcDoc tool.</div>"
	html += "<div class='definfo'><a href="+self.name+"_index.html>"+self.name+" Architechture Description</a></div>"
	html += "<div class='definfo'><a href="+self.name+"_isa.html>"+self.name+" Instruction Set and Formats</a></div>"
	html += ast.generateOtherHtml()
	self.index.write(html)

class HtmlSource():
    
    words = {'AC_ISA' : 'color="purple"', 'ISA_CTOR' : 'color="purple"',
	'set_decoder': 'color="blue"', 'set_asm' : 'color="blue"',
	'set_cycles' : 'color="blue"', 'cycle_range' : 'color="blue"',
	'ac_format' : 'color="green"', 'ac_instr' : 'color="green"',
	'ac_group' : 'color="purple"', 'ac_helper' : 'color="purple"',
	'ac_asm_map' : 'color="purple"', 'pseudo_instr' : 'color="purple"',
	'is_jump' : 'color="blue"', 'is_branch' : 'color="blue"',
	'cond' : 'color="blue"', 'delay' : 'color="blue"',
	'delay_cond' : 'color="blue"', 'behavior' : 'color="blue"',
	'AC_ARCH' : 'color="purple"', 'ARCH_CTOR' : 'color="purple"',
	'ac_tlm_port' : 'color="green"', 'ac_tlm_intr_port' : 'color="green"',
	'ac_cache' : 'color="green"', 'ac_icache' : 'color="green"',
	'ac_dcache' : 'color="green"', 'ac_mem' : 'color="green"', 'ac_isa': 'color="green"',
	'ac_reg': 'color="green"',  'ac_regbank' : 'color="green"', 
	'ac_format' : 'color="green"', 'ac_wordsize' : 'color="green"',
	'ac_fetchsize' : 'color="green"', 'ac_stage' : 'color="green"',
	'ac_pipe' : 'color="green"', 'set_endian' : 'color="green"',
	'bindTo' : 'color="green"', 'bindsTo' : 'color="green"', 
	'comment' : 'color="gray"', 'string' : 'color="brown"' }
    
    comment = False
    
    def __init__(self,name):
	self.name = name
	self.arch = name + '.ac'
	self.isa = name + '_isa.ac'
	self.generateHtml(self.arch)
	self.generateHtml(self.isa)
	
    def highlightText(self,line):
	
	if self.comment is True:
	    c = line.find('*/')
	    if c != -1:
		self.comment = False
	    
	    line = "<font " + self.words['comment'] + " >" + line + " </font>"
	    return line
	
	c = line.find('/*')
	if c != -1:
	    d = line.find('*/')
	    if d != -1: 
		text = line[c:d+2]
		rest = line[d+2:]
		self.comment = False
	    else: 
		text = line[c:] 
		rest = ""
		self.comment = True
	    comm = "<font " + self.words['comment'] + " >" + text + " </font>"
	    line = line[:c] + comm + rest
	    
	
	l = line.find('//')
	if l != -1 and c == -1:
	    comm = "<font " + self.words['comment'] + " >" + line[l:] + " </font>"
	    line = line[:l] + comm
	    
	s = line.find('"')
	if s != -1 and c == -1 and (s < l or l == -1):
	    t = line.find('"',s+1)
	    st = "<font " + self.words['string'] + " >" + line[s:t+1] + " </font>"
	    line = line[:s] + st + line[t+1:]
	    
	for word in self.words.keys():
	    i = line.find(word)
	    if (i > l and l != -1) or (i > c and c != -1):
		continue
	    elif i != -1:
		text = "<font "+ self.words[word] +" >" + word + " </font>"
		line = line.replace(word,text)
		break
		
	return line
	
	
    def generateHtml(self,filename):
	f = open(filename,'r')
	lines = f.readlines()
	html = filename.split('.')[0] + '_source.html'
	f = open(html,'w')
	
	html = "<html>\n<head>\n"
	html += "<link href='style.css' rel='stylesheet' type='text/css' />\n"
	html += "<title>"+ filename +" Source Code</title>\n</head>\n<body>\n"
	html += "<div class='definfo'>This documentation was automatically generated with AcDoc tool.</div>"
	html += "<div class='definfo'><a href="+self.name+"_index.html>"+self.name+" Architechture Description</a></div>"
	html += "<div class='definfo'><a href="+self.name+"_isa.html>"+self.name+" Instruction Set and Formats</a></div>"
	html += "<div class='definfo'><a href="+self.name+"_other.html>"+self.name+" Other Properties</a></div>"   
	html += "<div class='source'><ul>\n"
	for i in range(1,len(lines)+1):
	    lineno = "0"*(5-len(str(i))) + str(i)
	    html += "<li><a name=l"+str(i)+">"+lineno+"</a>&nbsp&nbsp&nbsp&nbsp "+self.highlightText(lines[i-1])+"</li>"
	html += "</ul></div>\n"
	html += "</body>\n</html>\n"
	f.write(html)
	f.close()

class acDoc(object):
    """ Main class, contains method for building acdoc, the AST and generating HTML code.
    """
    
    def __init__(self,code_info,comment_info,filename,instr):
	environ['arch'] = filename
	self.name = filename
	self.code = code_info
	self.comment = comment_info
	self.output = filename + ".html"
	self.instr = instr
	
    def generateHtml(self):
	HtmlIndex(self.name,self.ast[0])
	HtmlISA(self.name,self.ast[1])
	HtmlOther(self.name,self.ast[1])
	HtmlSource(self.name)
	
    def makeAST(self):
	""" Makes the AST based on the language Structure.
	"""
	self.ast = []
	# Here we implemented a sample case: Getting the architecture name
	processor = self.code['AC_ARCH']
	arch = ac_arch(processor)
	
	if 'ac_cache' in self.code:
	    tmp = self.code['ac_cache']
	    for i in tmp.keys():
		cache = ac_cache(i,tmp[i])
		arch.caches.append(cache)
	if 'ac_icache' in self.code:
	    tmp = self.code['ac_icache']
	    for i in tmp.keys():
		icache = ac_icache(i,tmp[i])
		arch.icaches.append(icache)
	if 'ac_dcache' in self.code:
	    tmp = self.code['ac_dcache']
	    for i in tmp.keys():
		dcache = ac_dcache(i,tmp[i])
		arch.dcaches.append(dcache)
	if 'ac_regbank' in self.code:
	    tmp = self.code['ac_regbank']
	    for i in tmp.keys():
		regbank = ac_regbank(i,tmp[i])
		arch.regbanks.append(regbank)
	if 'ac_mem' in self.code:
	    tmp = self.code['ac_mem']
	    for i in tmp.keys():
		mem = ac_mem(i,tmp[i])
		arch.mems.append(mem)			
	if 'ac_wordsize' in self.code:
	    tmp = self.code['ac_wordsize']
	    word = ac_wordsize(tmp)
	    arch.wordsize = word
	if 'ac_fetchsize' in self.code:
	    tmp = self.code['ac_fetchsize']
	    fetch = ac_fetchsize(tmp)
	    arch.fetchsize = fetch
	if 'ac_reg' in self.code:
	    tmp = self.code['ac_reg']
	    for i in tmp.keys():
		reg = ac_reg(i,tmp[i])
		arch.regs.append(reg)
	if 'AC_ARCH' in self.code:
		tmp = self.code['AC_ARCH']
		try:
		    comment = self.comment[tmp + '.ac']
		except KeyError:
		    pass
		else:
		    arch.comment = comment	
		
	isaname = self.code['ac_isa']
	endian = self.code['set_endian']
	bindlist = []
	if 'bindTo' in self.code:
	    binds = self.code['bindTo']
	    for i in binds.keys():
		bindlist.append((i,binds[i]))
	archctor = arch_ctor(isaname,endian,bindlist)
	arch.constructor = archctor
	
	if 'ac_stage' in self.code:
	    tmp = self.code['ac_stage']
	    for i in tmp.keys():
		stage = ac_stage(i)
		arch.stages.append(stage)		
	if 'ac_pipe' in self.code:
	    tmp = self.code['ac_pipe']
	    for i in tmp.keys():
		pipe = ac_pipe(i,tmp[i])
		arch.pipelines.append(pipe)
	
	isa = ac_isa(self.code['ac_isa'])
	
	if 'ac_helper' in self.code:
	    tmp = self.code['ac_helper']
	    helper = ac_helper(tmp)
	    isa.helper = helper
	
	if 'ac_format' in self.code:
	    tmp = self.code['ac_format']
	    for i in tmp.keys():
		ilist = []
		for inst in self.instr:
		    t = self.instr[inst]
		    if t == i:
			ilist.append(inst)
		format = ac_format(i,tmp[i],ilist)
		isa.formatlist.append(format)
		
	if 'ac_group' in self.code:
	    tmp = self.code['ac_group']
	    for i in tmp.keys():
		group = ac_group(i,tmp[i])
		isa.grouplist.append(group)
		
	if 'ac_asm_map' in self.code:
	    tmp = self.code['ac_asm_map']
	    for i in tmp.keys():
		asmmap = ac_asm_map(i,tmp[i])
		isa.maplist.append(asmmap)
		
	if 'ac_isa' in self.code:
		tmp = self.code['ac_isa']
		try:
		    comment = self.comment[tmp + '.ac']
		except KeyError:
		    pass
		else:
		    isa.comment = comment
	
	for i in self.instr:
	    instruction = ac_instr(i,self.instr[i])
	    commands = self.code[i]
	    if 'set_decoder' in commands:
		decoder = set_decoder(commands['set_decoder'])
		instruction.decoder = decoder
	    if 'set_asm' in commands:
		tmp = commands['set_asm']
		for i in tmp:
		    asm = set_asm(i)
		    instruction.asms.append(asm)
	    if 'delay' in commands:
		_delay = delay(commands['delay'])
		instruction.delays.append(_delay)
	    if 'cond' in commands:
		_cond = cond(commands['cond'])
		instruction.conds.append(_cond)
	    if 'delay_cond' in commands:
		delaycond = delay_cond(commands['delay_cond'])
		instruction.delay_conds.append(delaycond)
	    if 'behavior' in commands:
		_behavior = behavior(commands['behavior'])
		instruction.behaviors.append(_behavior)
	    if 'is_jump' in commands:
		isjump = is_jump(commands['is_jump'])
		instruction.isjumps.append(isjump)
	    if 'is_branch' in commands:
		isbranch = is_branch(commands['is_branch'])
		instruction.isbranchs.append(isbranch)
	    isa.instrlist.append(instruction)	    
	
	if 'pseudo_instr' in self.code:
	    tmp = self.code['pseudo_instr']
	    for i in tmp.keys():
		pseudo = pseudo_instr(i,tmp[i])
		isa.pseudolist.append(pseudo)
	
	self.ast.append(arch)
	self.ast.append(isa)
	
    def printInfo(self):
	""" Print acdoc attributes.
	"""
	print self.code
	print self.comment
	print self.output