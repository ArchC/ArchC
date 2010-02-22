# -*- coding: utf-8 -*-
"""	acParser.py
	
	Author: Eduardo Camargo

	This file contains the main class for the ArchC Language Parser.
	Basically, it's divided into 2 parts:
	    1. Parser Actions: methods for gathering information when a token
	    is found by the parser.
	    2. Parser Class: methods for parsing ArchC files, and all the
	    tokens and grammar rules are described as class attributes.

	Here we use the pyparsing lib, which is a different aproach compared
	to Bison-generated parsers. There's no need for states (such as 
	COMMENT, BLOCK, HEX), since it all can be described as a token combination
	using some of pyparsing pre-built types.

	There's also a type extension of the Literal class, called supLiteral,
	abbreviation for "Suppressed Literal", which is very useful, since there
	are a lot of tokens with significant grammatical value, but with no 
	information in it. So, what we do is: check for the token presence in the
	sentence, and then discard it afterwards, so we don't overload the info 
	sent to the next stages (Decoder and Doc Generator).
"""

from pyparsing import Literal, Word, Upcase, delimitedList, \
	Optional, alphas, nums, alphanums, ParseException, \
	Forward, oneOf, quotedString, ZeroOrMore, restOfLine, SkipTo, lineno, \
	col, commaSeparatedList, OneOrMore

import string
from acDoc import acDoc
from sys import argv

class parserActions():
	# Dictionaries
	__units = {'K' : 1024, 'M' : 1024 ** 2, 'G' : 1024 ** 3, 'T' : 1024 ** 4}
	__keywordConversions={'ARCH_CTOR' : 'AC_ARCH', 'ISA_CTOR': 'AC_ARCH', 'AC_ISA': 'AC_ARCH'}
	
	def __init__(self):
	    return

	def acError(self,message, str, loc):
	    print 'ERROR: %d, %d: %s' % (lineno(loc, str), col(loc, str), message)
	    
	def SetProcessorName(self,str, loc, tokens):
	    """ This method sets the processor name by putting an element in the 
		info dictionary.
	    """
	    acParser.info['ProcessorName'] = tokens[1]
	    return []
    
	def RemoveUnit(self,str, loc, tokens):
	    """ This action receives one or two tokens. If the second token is available,
	    it is considered the unit and the returned value is the first token as a number,
	    using the second token as a unit. Else, the returned value is the first token as a number
	    """
	    value = string.atoi(tokens[0])
	    if len(tokens) > 1:
		value = value * self.__units[tokens[1]]
	    return value
	    
	def ConvertFromHex(self,str, loc, tokens):
	    """ Converts the token from hexadecimal to decimal
	    """
	    result = int(tokens[0],16)
	    return result
	    
	def MakeList(self,str, loc, tokens):
	    """ Put the tokens in a list.
	    """
	    return [tokens]
	    
	def MakeTuple(self,str, loc, tokens):
	    """ Put the tokens in a tuple.
	    """
	    return tuple(tokens)
	    
	def AddSubItems(self,str, loc, tokens):
	    """ This method adds the subitens parsed to the info dictionary.
	    """
	    items = {}
	    keyword = tokens[0]
	    if keyword in acParser.info:
		items = acParser.info[keyword]
	    for item in tokens[1:]:
		symbol = item[0]
		if len(item) > 2:
		    value = item[1:]
		else:
		    value = item[1]
		if symbol in items:
		    self.acError('%s already declared' % symbol, str, loc)
		    exit(1)
		items[symbol] = value
		acParser.info[keyword] = items
	    return []
    
	def AddNewItem(self,str, loc, tokens):
	    """ Adds a new item to the info dictionary.
	    """
	    keyword = tokens[0]
	    value = tokens[1]
	    if keyword in acParser.info:
		self.acError('%s already declared' % keyword, str, loc)
	    acParser.info[keyword] = value
	    return []

	def CheckItem(self,str, loc, tokens):
	    """ This method checks if the item has been declared. If it has, it checkes for
		the value stored that must be equal. Otherwise it fails and exits the parser.
	    """
	    if tokens[0] in self.__keywordConversions:
		keyword = self.__keywordConversions[tokens[0]]
	    else:
		keyword = tokens[0]
	    value = tokens[1]
	    print keyword
	    if acParser.info[keyword] != value:
		self.acError('%s should have %s as parameter.' % (tokens[0], acParser.info[keyword], str, loc))
	    return []

	def PrintItem(self,str,loc,tokens):
	    """ Auxiliary action, to see how the parser collects the information on some tokens.
	    """
	    print "Token found: ",tokens
	    return []
	    
	def ParameterTuple(self,str,loc,tokens):
	    try:
		value = int(tokens[0])
	    except ValueError:
		value = tokens[0]
	    return ("Parameter",value)

	def AddItemList(self,str, loc, tokens):
	    """ Adds a new Item which contains a list of values
	    """
	    keyword = tokens[0]
	    items = { }
	    if keyword in acParser.info:
		items = acParser.info[keyword]
	    if isinstance(tokens[1],tuple):
		trash,value = tokens[1]
		i = 2
	    else:
		value = None
		i = 1
	    for symbol in tokens[i:]:
		if symbol not in items:
		    items[symbol] = value
		else:
		    self.acError('%s already declared' % symbol, str, loc)
		    exit(1)
	    acParser.info[keyword] = items
	    return []

	def AddInstr(self,str,loc,tokens):
	    """ Adds a pseudo instruction to the info dictionary
	    """
	    keyword = tokens[0]
	    pseudo = tokens[1]
	    instr_list = tokens[2:]
	    items = { }
	    if keyword in acParser.info:
		items = acParser.info[keyword]
	    if pseudo not in items:
		items[pseudo] = instr_list
	    else:
		tmp = items[pseudo]
		items[pseudo] = [tmp,instr_list]
	    acParser.info[keyword] = items
	    return []
	    
	def BindTuple(self,str,loc,tokens):
	    """ Add a bindTo instruction to the info dictionary
	    """
	    items = { }
	    key = tokens[0]
	    keyword = 'bindTo'
	    value = tokens[2]
	    if keyword in acParser.info:
		items = acParser.info[keyword]
	    if key not in items:
		items[key] = value
	    else:
		self.acError('%s already declared' % key, str, loc)
		exit(1)
	    acParser.info[keyword] = items
	    return []
	    
	def AddInstName(self,str,loc,tokens):
	    """ Adds an instruction name/type to the instruction list dictionary.
	    """
	    itype = tokens[1]
	    names = tokens[2:]
	    for name in names:
		if name not in acParser.instructions:
		    acParser.instructions[name] = itype
		else:
		    self.acError('%s already declared' % name, str,loc)
		    exit(1)
	    return []
	    
	def AddDoc(self,str,loc,tokens):
	    """ Adds documentation info on the dictionary.
	    """
	    __tags = ['\\author','\\brief','\\bug','\\date','\\details','\\end','\\note','\\see','\\since','\\todo','\\version','\\warning']
	    __file_tag = '\\file' + SkipTo(oneOf(__tags))
	    __file_tag.setParseAction(self.MakeTuple)
	    __doc_item = oneOf(__tags) + SkipTo(oneOf(__tags))
	    __doc_item.setParseAction(self.MakeTuple)
	    __doc = __file_tag + ZeroOrMore(__doc_item)
	    taglist =  __doc.parseString(tokens[0])

	    key,filename = taglist[0]
	    filename = filename.strip('\n ')
	    if filename not in acParser.doc_info:
		doc = { }
	    else:
		doc = acParser.doc_info[filename]
	    
	    for tag,value in taglist[1:]:
		tag = tag[1:]
		doc[tag] = value
		
	    acParser.doc_info[filename] = doc
	    
	def AddRegbank(self,str,loc,tokens):
	    """ Adds a register bank to the dictionary
	    """
	    keyword = tokens[0]
	    if keyword in acParser.info:
		items = acParser.info[keyword]
	    else:
		items = { }
		
	    if isinstance(tokens[1],tuple):
		trash,p = tokens[1]
		i = 2
	    else:
		i = 1
		p = None
	    for [name,value] in tokens[i:]:
		if name in items:
		    self.acError('%s already declared' % name, str,loc)
		else:
		    items[name] = (value,p)
	    acParser.info[keyword] = items
	    return []

	def AddOccur(self,str,loc,tokens):
	    acParser.occur[tokens[0]] = lineno(loc,str)

	    
class supLiteral(Literal):
	"""
	This class defines suppressed literals, a very common
	element of the ArchC language parser.
	"""
	def __init__(self,tok):
	    super(supLiteral,self).__init__(tok)
	    # Redefinition of the error message
	    self.errmsg = "Expected suppressed literal "+ self.name

	# This is the method which suppresses the token
	def postParse( self, instring, loc, tokenlist ):
	    return []	

class acParser(object):
	""" Main parser class that contains tokens and rules definitions, as well
	    as the parsing methods.
	"""
	info = { }			# Stores all relevant info from the code
	doc_info = { }			# Stores documenting-only info (from comments)
	actions = parserActions()	# Parsing Actions Object
	instructions = { }		# Instructions names/formats dictionary
	occur = { }
	
	def __init__(self):
	    return
	
	# Comments
	__comment1 = supLiteral('//') + restOfLine.suppress()
	__comment2 = supLiteral('/*') + SkipTo(supLiteral('*/'),include = True)

	# Here we have a special type of comment, labeled "doc_comment). The difference
	# of this kind of comment is that the info on it is not discarded, but parsed
	# by specific rules and given as an source for the acDoc module.
	__doc_comment = supLiteral('/**\n') + SkipTo(supLiteral('*/'),include=True)
	__doc_comment.setParseAction(actions.AddDoc)
	
	__comment = __doc_comment | __comment1 | __comment2

	# List of suppressed literals
	__colon = supLiteral(':')
	__lPar = supLiteral('(')
	__rPar = supLiteral(')')
	__lBrace = supLiteral('{')
	__rBrace = supLiteral('}')
	__lBrack = supLiteral('[')
	__rBrack = supLiteral(']')
	__semiColon = supLiteral(';')
	__comma = supLiteral(',')
	__equal = supLiteral('=')
	__quotes = supLiteral('"')
	__percent = supLiteral('%')
	__lThan = supLiteral('<')
	__gThan = supLiteral('>')
	__dot = supLiteral('.')
	__plus = supLiteral('+')
	__or = supLiteral('|')

	# Parameters
	# Defined here are basic tokens such as variable names (symbol), numbers, size (eg. 32K),
	# endian (little or big)
	__unit = ['K','M','G','T']
	# ID = [a-zA-Z][_a-zA-Z0-9]*
	__symbol = Word(alphas, alphanums + '_').setName('symbol')
	__str = __quotes + SkipTo(__quotes,include=True)
	# INT = [0-9]+
	__int = Word(nums)
	# HEX = (0x | 0X) [0-9a-fA-F]+
	__hexnumber = (supLiteral('0x') | supLiteral('0X')) + Word(nums+'abcdefABCDEF')
	__hexnumber.setName('hexadecimal number')
	__hexnumber.setParseAction(actions.ConvertFromHex)
	# NUM = ([0-9]+ | HEX)
	__number = (__int ^ __hexnumber).setName('number')
	# SIZE = NUM + (K | M | G | T | k | m | g | t)
	__size = (__number + oneOf(__unit,caseless = True)*(0,1)).setName('size')
	__size.setParseAction(actions.RemoveUnit)
	#ENDIAN = "(big | little)"
	__endian = (__quotes + (Literal('big') | Literal('little')) + __quotes).setName('endian')
	#FILENAME = "[any char]"
	__filename =  (__str).setName('filename')
	
	__bindTo = __symbol + __dot + (Literal('bindTo') | 'bindsTo') + __lPar + __symbol + __rPar + __semiColon
	__bindTo.setParseAction(actions.BindTuple)
	
	# ID[size] or ID : size
	__ac_item = (__symbol + __lBrack + __size + __rBrack) ^ (__symbol + __colon + __size)
	__ac_item.setParseAction(actions.MakeList)

	# Reserved Words
	# Here we define most of the rules in ArchC. These are all declaration statements

	__cache_types = ['ac_cache','ac_icache','ac_dcache']
	__ac_cache = oneOf(__cache_types) + delimitedList(__ac_item) + __semiColon
	__ac_cache.setParseAction(actions.AddSubItems)	
	
	__ac_mem = 'ac_mem' + delimitedList(__ac_item) + __semiColon
	__ac_mem.setParseAction(actions.AddSubItems)
	__ac_wordsize = 'ac_wordsize' + __size + __semiColon
	__ac_wordsize.setParseAction(actions.AddNewItem)
	__ac_isa = 'ac_isa' + __lPar + __filename + __rPar + __semiColon
	__ac_isa.setParseAction(actions.AddNewItem)

	__assign_width = __lThan + (__number) + __gThan
	__assign_width.setParseAction(actions.ParameterTuple)
	__ac_regbank = 'ac_regbank' + Optional(__assign_width) + delimitedList(__ac_item) + __semiColon
	__ac_regbank.setParseAction(actions.AddRegbank)
	
	__set_endian = 'set_endian' + __lPar + __endian + __rPar + __semiColon
	__set_endian.setParseAction(actions.AddNewItem)

	__reg_parm = __lThan + (__symbol ^ __number) + __gThan
	__reg_parm.setParseAction(actions.ParameterTuple)
	__ac_reg = 'ac_reg'+ Optional(__reg_parm) + delimitedList(__symbol) + __semiColon
	__ac_reg.setParseAction(actions.AddItemList)

	__ac_fetchsize = 'ac_fetchsize' + __size + __semiColon
	__ac_fetchsize.setParseAction(actions.AddNewItem)
	
	__ac_stage = 'ac_stage' + delimitedList(__symbol) + __semiColon
	__ac_stage.setParseAction(actions.AddItemList)
	__ac_pipe = 'ac_pipe' + __symbol + __equal + __lBrace + delimitedList(__symbol) + __rBrace + __semiColon
	__ac_pipe.setParseAction(actions.AddInstr)
	
	__ac_port = 'ac_tlm_port' + delimitedList(__ac_item) + __semiColon
	__ac_port.setParseAction(actions.AddSubItems)
	__ac_intr_port = "ac_tlm_intr_port" + delimitedList(__ac_item) + __semiColon
	__ac_intr_port.setParseAction(actions.AddSubItems) 
	
	# New ac_asm comment declarations
	__asm_com_dec = "assembler" + __dot + (Literal("set_comment") | "set_line_comment" ) + __lPar + __str + __rPar + __semiColon
	
	__map_list = __lBrack + __number + __dot*(2,2) + __number + __rBrack
	__map_body = ((delimitedList(__str) + __equal + __number) ^ (__str + __map_list + Optional(__str) + __equal + __map_list) ^ \

		    (__map_list + __str + __equal + __map_list)) + __semiColon
	__map_body.setParseAction(actions.MakeList)
	__ac_asm_map = 'ac_asm_map' + __symbol + __lBrace + OneOrMore(__map_body) + __rBrace
	__ac_asm_map.setParseAction(actions.AddInstr)
    
	# ac_format Definition
	__ac_formatField = ((__percent + __symbol) | __hexnumber) + __colon + __number + Optional(__colon + 's')
	__ac_formatField.setParseAction(actions.MakeTuple)
	__options = OneOrMore(__ac_formatField)
	__options.setParseAction(actions.MakeTuple)
	__ac_formatField2 = __lBrack + __options + __or + __options + __rBrack
	__ac_formatField2.setParseAction(actions.MakeTuple)
	__ac_formatList = OneOrMore(__ac_formatField | __ac_formatField2)
	__ac_formatItem = __symbol + __equal + __quotes + __ac_formatList + __quotes
	__ac_formatItem.setParseAction(actions.AddOccur)
	__ac_formatItem.addParseAction(actions.MakeList)
	__ac_format = 'ac_format' + delimitedList(__ac_formatItem) + __semiColon
	__ac_format.setParseAction(actions.AddSubItems)
	__ac_format.setName('ac_format')

	__ac_instr = 'ac_instr' + __lThan + __symbol + __gThan + (delimitedList(__symbol)) + __semiColon
	__ac_instr.setParseAction(actions.AddInstName)
    
	__reglist = __comma + delimitedList(__symbol + (Optional((__equal | __plus) + (__symbol | __number | __str))))
	__set_asm = __symbol + __dot + 'set_asm' + __lPar + __str + Optional(__reglist) + __rPar + __semiColon
	__set_asm.setParseAction(actions.AddInstr)

	__decoderItem = __symbol + __equal + __number
	__decoderItem.setParseAction(actions.MakeTuple)
	__set_decoder = __symbol + __dot + 'set_decoder' + __lPar + delimitedList(__decoderItem) + __rPar + __semiColon
	__set_decoder.setParseAction(actions.AddInstr)
	
	__set_cycles = __symbol + __dot + 'set_cycles' + __lPar + __int + __rPar + __semiColon
	__set_cycles.setParseAction(actions.AddInstr)
	__cycle_range = __symbol + __dot + 'cycle_range'+ __lPar + __int + Optional(__comma + __int) + __rPar + __semiColon
	__cycle_range.setParseAction(actions.AddInstr)
	
	__pseudo_body = __str + __semiColon
	__pseudo_instr = 'pseudo_instr' + __lPar + __str + __rPar + __lBrace + OneOrMore(__pseudo_body) + __rBrace
	__pseudo_instr.setParseAction(actions.AddInstr)
	
	__opt_list = ['is_jump','is_branch','cond','delay','delay_cond','behavior']
	__opt_instr = __symbol + __dot + oneOf(__opt_list) + __lPar + restOfLine #SkipTo(__rPar + __semiColon,include=True)
	__opt_instr.setParseAction(actions.AddInstr)
	
	__ac_helper = 'ac_helper' + __lBrace + SkipTo(__rBrace + __semiColon,include=True)
	__ac_helper.setParseAction(actions.AddNewItem)
	__ac_group = 'ac_group' + __symbol + __lBrace + delimitedList(__symbol) + __rBrace + __semiColon
	__ac_group.setParseAction(actions.AddInstr)

	# Language constructors

	__storagedecs = __ac_cache | __ac_mem | __ac_regbank | __ac_reg | __ac_port | __ac_intr_port
	__ac_arch_commands = ZeroOrMore(__storagedecs | __ac_wordsize | __ac_isa | __set_endian | __ac_fetchsize | __ac_stage | __ac_pipe | __ac_format | __bindTo)
	__ac_isa_commands = ZeroOrMore(__ac_format | __set_asm | __set_decoder | __ac_instr | __pseudo_instr | __ac_asm_map | \
			    __opt_instr | __ac_helper | __ac_group | __set_cycles | __cycle_range | __asm_com_dec)

	__ARCH_CTORHeader = 'ARCH_CTOR' + __lPar + __symbol + __rPar
	__ARCH_CTORHeader.setParseAction(actions.CheckItem)
	__ARCH_CTORBody = __lBrace + __ac_arch_commands + __rBrace + __semiColon
	__ARCH_CTOR = __ARCH_CTORHeader + __ARCH_CTORBody
	
	__AC_ARCHHeader = 'AC_ARCH' + __lPar + __symbol + __rPar
	__AC_ARCHHeader.setParseAction(actions.AddNewItem)
	__AC_ARCHBody = __lBrace + __ac_arch_commands + __ARCH_CTOR + __rBrace + __semiColon
	__AC_ARCH = __AC_ARCHHeader + __AC_ARCHBody
	__AC_ARCH.ignore(__comment)
	
	__ISA_CTORHeader = 'ISA_CTOR' + __lPar + __symbol + __rPar
	__ISA_CTORHeader.setParseAction(actions.CheckItem)
	__ISA_CTORBody = __lBrace + __ac_isa_commands + __rBrace + __semiColon
	__ISA_CTOR = __ISA_CTORHeader + __ISA_CTORBody
	
	__AC_ISAHeader = 'AC_ISA'  + __lPar + __symbol + __rPar
	__AC_ISAHeader.setParseAction(actions.CheckItem)
	__AC_ISABody = __lBrace + __ac_isa_commands + __ISA_CTOR + __rBrace + __semiColon
	__AC_ISA = __AC_ISAHeader + __AC_ISABody
	__AC_ISA.ignore(__comment)


	def testParser(self,parserStr):
	    """ This a test method for specific rules (or debugging, if needed)
	    """
	    print "------------------Testing parser----------------------"
	    print self.__ac_regbank.parseFile(parserStr)
	    print self.info
	    print '--------------------End of test ----------------------'

	def ParseFile(self, fileName):
		""" Main parser method. Receives an input file, and runs it through the parser.
		"""
		try:
		    self.__AC_ARCH.parseFile(fileName)
		    self.fileName = self.info['AC_ARCH']
		    if 'ac_isa' in self.info:
			# Now, the same procedure is done with the ISA
			self.__AC_ISA.parseFile(self.info['ac_isa'])
		except ParseException, err:
		    print "Error"
		    print err.msg
		    print err
	    
	def docFile(self):
	    """ If the parsing phase was successful, the info gathered can
		now be used by the acDoc module to generate HTML documentation
		for the code.
	    """
	    self.ac_doc = acDoc(self.info,self.doc_info,self.fileName,self.instructions)
	    self.ac_doc.makeAST()
	    self.ac_doc.generateHtml()
