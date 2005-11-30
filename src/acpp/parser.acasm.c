/* 
    ArchC parser (acasm module) - Copyright (C) 2002-2005  The ArchC Team

    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.
*/

/********************************************************/
/* parser.acasm.c: ArchC parser (acasm module)  .       */
/* Author: Alexandro Baldassin                          */
/* Date: 01-06-2005                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

/*

Overview - Internal aspects

Parser x Tools

  There are mainly two aspects of the ArchC language you should be aware of:
parser and tools. Although you may think of them as two different things, the
way it's implemented at the moment looks like a single 'beast'. The ideal
solution would be to split the parser into a unique and independent module: the
information collected would then be saved into an intermediate
representation. The tools would use this intermediate representation to
generate their files and stuff. For old reasons, the parser and tools views of
the language are someway the same. This has some bad side effects, but it saved
us time in the development and we are slowly focusing in the intermediate
representation.  This module represents the parser structures and functions
used by the acasm tool. Acasm is a generator of an assembler based in the GNU
as. The structures we'll find here are mainly dependent of the acasm tool, yet
it's used by the parser as well. The parser collects the information found in
the ArchC .ac files and store them in these structures (the acasm specific
informations). The assembler specific generator module can then use these
structures to create the source files which will build an assembler for that
architecture.


Philosophy

  Each increment we make in the parser of the language is tied to a tool
necessity. So the way we're starting to split things is: put the parser
relative structures and functions in the 'parser.<tool>.*' files and the tool
specific files in their respective directories. At this moment, acasm is the
first tool to use this philosophy. We hope this will prove itself worthy so we
can adapt the other tools (acsim, accsim, ...) as well.  The parser (which uses
Bison) and the acasm module should include this module header file so that it
can use the structures and functions provided. Note that -NO- global shared
variables are used but interface functions where needed.  The parser fills in
the structures calling some interface functions provided (the ones with the
prefix 'acpp_asm') and the acasm module uses those informations by calling
other group of functions (the ones with the prefix 'ac_asm'). Note also that
all structures start with the ac_asm prefix (their use are someway 'shared' by
the parser and tool by means of the interface functions). There might be some
structures (like ac_dec_insn) which needs to be shared by more then one
tool. Those structures are being stored in their first and original modules at
this moment.


The 'acasm parser' (information useful to developers)
  
  I decided to used the term 'acasm parser' instead of only 'parser' since,
already stated in the Parser x Tools section, this parser functions are
relative only to those specific to the acasm tool. From now on, whenever
'parser' is used, it'll mean acasm parser.  The parser is composed of basicly 3
groups. Each group is relative to one acasm keyword of the ArchC language. They
are: 'ac_asm_map', 'set_asm' and 'pseudo_instr' keywords. Maybe it was a better
programming style to split this file into 3, each of them dealing with one
keyword. But since it's a first version, it's all grouped here.  All parser
interface functions use a kind of state machine to do its processing. So there
is a sequence to call the functions, and each one updates its internal
state. These states and sequence ordering are described bellow for each of the
keywords. A basic information is given for every keyword, stating the general
idea of the implementation. Detailed description about the implementation can
be found through the source code.

*******************************
 -> ac_asm_map
*******************************   

  syntax: 
    <asm_map> ::= 'ac_asm_map' <marker> '{' (<symbol_mapping>)*';' '}'

    <marker> -a valid identifier used as a mapping ID- must be unique
    a valid identifier is  [a-zA-Z][_a-zA-Z0-9]* (declared in the .lex file)

    <symbol_mapping> ::= <symbol>(,<symbol>)* '=' <value> |                         
                         <symbol>'['<value>'..'<value>']' '=' '['<value>'..'<value>']' |
                         '['<value>'..'<value>']'<symbol> '=' '['<value>'..'<value>']' |
			  <symbol>'['<value>'..'<value>']'<symbol> '=' '['<value>'..'<value>']'

    <symbol> ::= '"'<symbol_id>'"'
    <symbol_id> -any sequence of valid ASCII chars- (valid chars are those accepted by is_map_symbol_* functions)

    <value> -any non-negative integer-
    <value2> -any integer-

  valid example:
 
      ac_asm_map reg {
         "$"[0..31] = [0..31];
         "$zero" = 0;
         "$v"[0..1] = [2..3];
         "$t"[8..9] = [24..25];
         "$s"[0..7] = [16..23];    
         "k"[0..1] = [26..27];
         "$gp" = 28;
         "$s8", "$fp" = 30;       
         "e"[1..3]"i" = [10..8];
      }

  acasm Info:

  A list of the mappings declared is stored in the 'mapping_list' variable. It
can be retrieved through the 'ac_asm_get_mapping_list()' function. It's up to
the caller to get the info they need in the list, just be sure you don't change
it ;)


  Basic Parser Info:
    
  Upon finding the 'ac_asm_map' keyword, the parser should call
'acpp_asm_create_mapping_block()' with the ID as an argument. This will check
for redefinition of the ID and add it to the internal list. After this, each
<symbol> found in the right side of the attribution must be added calling
either 'acpp_asm_add_mapping_symbol()' or 'acpp_asm_add_mapping_symbol_range()'
depending if the symbol include a range [] or not. The value (or the range of
values) can then be assigned through the 'acpp_asm_add_symbol_value()'
function.
  The dependency among the calls to the functions is:

   Step 1 - call acpp_asm_create_mapping_block() to create a mapping block
   Step 2 - call acpp_asm_add_mapping_symbol() or
            acpp_asm_add_mapping_symbol_range() to stack symbol definitions for
            that block
   Step 3 - call acpp_asm_add_symbol_value() to assign a value (or a range of
            values) to the symbol(s) stacked
   Step 4 - either add more symbols going to step 2 or create another mapping
            block going to step 1


******************************* 
 -> set_asm
******************************* 

  set_asm is tied up to an 'ac_instr' type. It's the type used by the ArchC
language so that instructions can be created.  Every insn declared by
'ac_instr' can use a property called 'set_asm' to declared its assembly syntax
and encoding scheme.  'set_asm' resembles the standard C function 'scanf':
there's a string with literal and formatting characters (those starting with
the '%' char) and a list of arguments for each of the formatting
characters. The syntax string is not parsed by the bison itself; it uses some
of the functions implemented here. The general format of a set_asm declaration
is:

    <archc_instr>'.''set_asm' '(' <syntax_string> { ',' <list_of_arguments> } ')' ';'

    <syntax_string> is made up of literal and formatting characters. Literals
    are those characters that should be matched literally in the input source
    code whereas formatting characters always start with the symbol '%'
    followed by an identifier. A valid identifier is the one that can be
    created using [a-zA-Z][_a-zA-Z0-9]* and are declared using 'the ac_asm_map'
    keyword or else it's of a built-in type.  Some things you should be aware
    of when talking about syntax strings are:
       
     . the first whitespace found in the string splits it in the 'mnemonic' and
       'operands' parts; if there is no whitespace then there is no operand string 
       for that insn syntax;
        example: 
	  set_asm("add x1, x2");  
	    "add"   -> mnemonic
	    "x1,x2" -> operands

     . you can put the format identifier between the '[' ']' character when using 
       the '%' character;
        example:
	  set_asm("add %[register],%register", ...);
	    both '%[register]' and '%register' have the same effect

     . formatting string in the mnemonic part of the string has a different meaning: 
       it tells the acasm to expand that in the symbols defined under the format 
       identifier (only identifiers created through 'ac_asm_map' are valid);
        example:
          ac_asm_map annul {
	    "" = 0;
            ",a" = 1;   
          }

          set_asm("bvs%[anul] %expRW", an, ...);
            the acasm tool will expand it in:
             "bvs   %expRW" (encoding the format field 'an' with the value 0);
	     "bvs,a %expRW" (encoding the format field 'an' with the value 1);

     . in operand strings, every sequence of the chars [a-zA-Z0-9_$.] are grouped as a 
       single token. All other valid characters are also a token. Among tokens it's not 
       necessary to specify whitespaces, they are implict.
        example:
         the following declarations are the same:

          set_asm("add %reg,[%reg]", ...);
          set_asm(" add %reg , [  %reg]", ...);

  	 the next one is not valid since there should be a valid identifier following 
         the first '%' char:
      
          set_asm("add % reg, %reg", ...);
	       
     . some characters cannot be part of the string. Those are described by the 
       is_mnemonic_* and is_operand_* functions. Some characters can appear in the mnemonic 
       part but not in the operand, and vice-versa.
                    
    <list_of_arguments> should match one of the fields declared through
    ac_format under the instruction which its assembly syntax is being
    specified. To every marker specified in the syntax string there should be
    one argument. The first argument found is relative to the first marker; the
    second argument is relative to the second marker and so. A constant
    argument is the one which can be assigned to a constant integer or symbol
    at the declaration time. They don't need any marker in the syntax string
    
      Example:
        ("add %reg, %imm", rs, imm)   -> '%reg's argument is 'rs' and '%imm' argument is 'imm
        ("add %reg", rs, imm=10)      -> here 'imm=10' is a constant argument


  Acasm Info:

  A list of the insn syntaxes declared are stored in the 'asm_insn_list'
variable. It can be retrieved through the 'ac_asm_get_asm_insn_list()'
function. It's up to the caller to get the info they need in the list, just be
sure you don't change it ;)


  Basic Parser Info:

  When a 'set_asm' directive is found in the ArchC source file and its syntax
should be checked, you must call the init function 'acpp_asm_new_insn()'. It
initializes some internal states. After that, syntax strings are parsed
through a calling to the function 'acpp_asm_parse_asm_string()'. Arguments and
constant arguments are processed by calling 'acpp_asm_parse_asm_argument()' or
'acpp_asm_parse_const_asm_argument()'. To insert the insn to the list, call
'acpp_asm_end_insn()'. It's the last step when parsing the whole 'set_asm'
stuff. Those functions work by creating internal representation of the strings
being parsed. end_insn is responsable for creating the final asm string and
insert it into the asm_insn_list.
  The dependency among the calls to the functions is:

   Step 1 - call acpp_asm_new_insn() before any other function, to initialize 
            internal states
   Step 2 - call acpp_asm_parse_asm_insn() to parse the syntax string
   Step 3 - either call acpp_asm_parse_asm_argument() or 
            acpp_asm_parse_const_asm_argument() to process the each argument
   Step 4 - call acpp_asm_end_insn() to create a new ac_asm_insn and insert it 
            in the list of insns


******************************* 
 -> pseudo_instr
******************************* 

  syntax:
    <pseudo_insn> ::= 'pseudo_instr' '(' <pseudo_declaration> ')' '{' {<pseudo_member> ';'}*1 '}'

    <pseudo_declaration> is a string of almost the same type of <syntax_string>. 
      Markers cannot be used in the mnemonic string.

    <pseudo_member> is a string with the insn syntaxes that should be executed 
      when <pseudo_declaration> is found. This string follows the same basic 
      syntax of <syntax_string>: mnemonic string cannot use markers. Macro 
      substitutions are made using an integer from 0-9 right after the '%' 
      character. 0 corresponds to the first marker in the <pseudo_declaration>, 
      1 to the second, and so on...

      example:
    
      pseudo_instr("li %reg, %imm") {
        "lui %0, \%hi(%1)";
        "ori %0, %0, %1";
      }

      When processing the macro insn 'li %reg, %imm', the assembler will 
      validate and evaluate the %reg and %imm fields. Then it will execute 
      the following two insns: 'lui' and 'ori'. %0 will be replaced by the 
      evaluated %reg, and %1 will be replaced by the evaluted %imm field in 
      the 'lui' insn.
      Note: Theorically a <pseudo_member> can be another pseudo_instr. 
        However it's not tested yet.


  acasm Info:

  A pseudo insn is also stored in the 'asm_insn_list'. However, some fields of
the structure ac_asm_insn has some fixed values.  'mnemonic' and 'operand'
fields store the mnemonic and operands strings of <pseudo_declaration>. 'insn'
field is always NULL since there is no ac_dec_instr attached to a
pseudo. 'const_image' is always 0 since pseudo-ops don't have
arguments. 'pseudo_list' is a string list with all <pseudo_member> as declared
in the ArchC source file. 'num_pseudo' is the number of pseudo members in the
'pseudo_list' field.


  Basic Parser Info:

 'acpp_asm_parse_asm_insn()' is also used to parse <pseudo_declaration> setting
the flag 'is_pseudo' to 0 (like a native insn).  Only after that one should
call 'acpp_asm_new_pseudo()'. To insert each <pseudo_member>, call
'acpp_asm_add_pseudo_member()'. It will insert them in the pseudo_list. To
finish, call 'acpp_asm_end_insn()' to insert it in the asm_insn_list.
  The dependency among the calls to the functions is:
       
   Step 1 - call acpp_asm_parse_asm_insn() to parse the base pseudo-op string 
            (<pseudo_declaration>)
   Step 2 - call acpp_asm_new_pseudo() to initialize internal states in 
            pseudo-op processing
   Step 3 - call acpp_asm_add_pseudo_member() for each insn of the pseudo 
            declaration
   Step 4 - call acpp_asm_end_insn() to insert all in the asm_insn_list




Some conventions:

- acasm specific parser structures use the prefix string 'ac_asm' 
  (that's because the decoder structures use 'ac_dec')

- there are no global variables shared among modules. Interface functions are 
  used instead. Functions which are used by the parser use the prefix 'acpp_asm' 
  whereas those used by the Acasm tool or this module itself use the prefix 
  'ac_asm'.

- where an error might raise, an interface function will use a variable (error_msg) 
  to store its message and will return 0. The caller is responsable for allocating 
  memory for error_msg before the call is made.

- variables and functions internal to this module uses the C -STATIC- qualifier.


*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "ac_decoder.h"
#include "parser.acasm.h"


/* 
  ac_asm_map variables 
*/

static ac_asm_map_list *mapping_list = NULL;       /* map list header pointer */
static ac_asm_map_list *mapping_list_tail = NULL;  /* tail pointer */

/* each mapping element has a collection of symbol-value mapping.
this variable holds the list of the symbol-value mapping being parsed for the
current mapping element.
*/
static ac_asm_symbol **map_symbol_list = NULL;    
static ac_asm_symbol *map_symbol_list_tail = NULL;  /* pointer to the tail */

/* a list of the symbol names being parsed by a single symbol-value mapping */
static strlist *symbol_name_list;  
static strlist *symbol_name_list_tail;  /* pointer to the tail */



/*
  set_asm variables
*/

/* 
   acpp_asm_parse_asm_str(), acpp_asm_parse_asm_argument() and acpp_asm_parse_const_asm_argument()
uses these internal buffer to store the building of the final asm string done in acpp_asm_end_insn()
*/
static char formatted_asm_str[200];
static char formatted_arg_str[100];
static char formatted_pseudo[200];  /* used by add_pseudo_member() */

/* Variables to keep track of the numbers of arguments necessary and found */
static int num_args_expected;
static int num_args_found;

static int in_error;

/* list of insn asm syntax - Acasm tool uses this list to generate its assembler */
static ac_asm_insn *asm_insn_list = NULL;

/* Keeps track of mnemonic's marker */
static ac_asm_map_list *mnemonic_marker = NULL;
static ac_dec_field *mnemonic_marker_field = NULL;

/* when const fields are used, this variable will store their encoding for the current insn */
static long int c_image;



/*
  pseudo_instr variables
*/
static long num_pseudo_insn = 0;      /* number of insns a pseudo-op is made up */
static strlist *pseudo_list = NULL;   /* the pseudo list for the current pseudo declaration */



/* variables taken from the bison file */
extern ac_dec_format *format_ins_list;   /* list of formats declared */


/*
 acasm interface functions
*/

ac_asm_insn* ac_asm_get_asm_insn_list() { return asm_insn_list; }
ac_asm_map_list* ac_asm_get_mapping_list() { return mapping_list; }



unsigned int ac_asm_encode_insn_field(unsigned int field_value, ac_dec_format *pformat, ac_dec_field *pfield) {

  // TODO: see if the 'val' field can accept constant values (this is not being checked atm)
  unsigned int mask1 = 0xffffffff;
  unsigned int mask2 = 0xffffffff;
  unsigned int return_value;

  mask1 <<= (pformat->size-(pfield->first_bit+1));
  mask2 >>= pfield->first_bit+1-pfield->size;
  return_value = ((field_value << (pformat->size-(pfield->first_bit+1)) & (mask1 & mask2)));

  return return_value;
}




/*
 Internal Helper functions
*/



/*
  This looks for the first occurence of 'symbol' in the mapping list.

  Note that some symbol may be declared several times in different map blocks.
  This function returns only the first occurence as it's found in the list.
*/
static int get_symbol_value(char *symbol, long int *symbol_id) {

  long int symbol_value = -1;
  ac_asm_map_list *ml = mapping_list;

  int found = 0;
  while (ml != NULL && !found) {

    ac_asm_symbol *sl = ml->symbol_list;

    while (sl != NULL) {
      if (!strcmp(sl->symbol, symbol)) {
	symbol_value = sl->value;
	found = 1;
	break;
      }	
      sl = sl->next;
    } 

    ml = ml->next;
  }

  *symbol_id = symbol_value;

  return found;
}


/*
  Searchs for the marker string 'marker' in the mapping_list. 
  If not found, NULL is returned.
*/
static ac_asm_map_list *find_mapping_marker(char *marker) {

  ac_asm_map_list *t_ml = mapping_list;

  while (t_ml != NULL) {

    if (!strcmp(t_ml->marker, marker)) return t_ml;

    t_ml = t_ml->next;
  }

  return NULL;
}


/* 
  Searchs for a symbol definition 'symbol' in the current mapping block.
*/
static ac_asm_symbol *find_mapping_symbol(char *symbol) {

  ac_asm_symbol *t_sl = *map_symbol_list;

  while (t_sl != NULL) {

    if (!strcmp(t_sl->symbol, symbol)) return t_sl;

    t_sl = t_sl->next;
  }

  return NULL;
}



/*
  Parsing functions

  Following is a list of functions to valid some strings (mnemonics, operand, markers, ...)
*/


/*
 A map symbol name may not include:
   . spaces (isspace())
   . ;  (statement delimiter in GNU As - can be changed)  
   . #  (commentary in GNU As - can be changed)
   . '  (single quote)
   . / * (C start comment block)
   . * / (C end comment block)

 Note:
  Be careful when defining symbols for mnemonic expansions. Some characters like '.' may
cause problems with the GNU as assembler.
   
*/
inline static int is_map_symbol_name(char *s) {
  if (isspace(*s) || *s == ';' || *s == '\''
      || (*s == '/' && *(s+1) == '*')
      || (*s == '*' && *(s+1) == '/') ) 
    return 0;

  return 1;
}

/* empty symbols ("") are valid! */
inline static int is_map_symbol_begin(char *s) {
  if (*s != '\0') return is_map_symbol_name(s);
  return 1;
}

inline static int is_map_symbol_end(char *s) {
  return is_map_symbol_name(s);
}



/*
 Validates ArchC identifiers

 This should be equal to the ID in the lex file, but is enough the way it is (no need for the _begin 
 version)
*/
inline static int is_identifier(char *s) {
  return ((*s >= 'a' && *s <= 'z') 
	  || (*s >= 'A' && *s <= 'Z') 
	  || (*s >= '0' && *s <= '9') 
	  || *s == '_');
}



/*
 Validates mnemonics and pseudo-mnemonics

*/

inline static int is_mnemonic_begin(char *s) {
  return (((*s >= 'a') && (*s <= 'z')) 
         || ((*s >= 'A') && (*s <= 'Z')) 
	 || (*s == '_') || (*s == '%'));
}

inline static int is_mnemonic_name(char *s) {
  if ( *s == ';' || *s == '\''
      || (*s == '/' && *(s+1) == '*')
      || (*s == '*' && *(s+1) == '/') ) 
    return 0;

  return 1;
}


/*
  Almost the same as the mnemonics version but the '%'.
*/
inline static int is_pseudo_mnemonic_begin(char *s) {
  return (((*s >= 'a') && (*s <= 'z')) 
         || ((*s >= 'A') && (*s <= 'Z')) 
	 || (*s == '_'));
}

inline static int is_pseudo_mnemonic_name(char *s) {
  if ( *s == ';' || *s == '#' || *s == '\'' || *s == '%' 
      || (*s == '/' && *(s+1) == '*')
      || (*s == '*' && *(s+1) == '/') ) 
    return 0;

  return 1;
}

inline static int is_operand_string(char *s) {
  return is_mnemonic_name(s);
}

inline static int is_group_token(char *s) {
  if (*s == '$' || *s == '.' || is_identifier(s)) return 1;

  return 0;
}


/*
  The idea is to use a 32 bit value where each bit works as a flag to one
of the base built-in's or of a modifier.

bit 0 - 'exp' flag
bit 1 - 'addr' flag
bit 2 - 'imm' flag
bit 3 - 'rel' modifier flag
and so on..

 Last value is of type invalid_modifier which indicates that there is a base
marker but the modifiers are invalid.

 Use the mask to test each individual bit. 
*/


#define BI_NONE             0
#define BI_BASE_EXP_MASK    1
#define BI_BASE_ADDR_MASK  (1 << 1)
#define BI_BASE_IMM_MASK   (1 << 2)
#define BI_MD_REL_MASK     (1 << 3)
#define BI_MD_HI_MASK      (1 << 4)
#define BI_MD_LO_MASK      (1 << 5)
#define BI_MD_AL_MASK      (1 << 6)
#define BI_MD_INVALID_MASK (1 << 10)

/*

 Bases:
        'exp'
	'addr'
	'imm'

 Modifiers:
        'H'[n][c][s|u] -> signed, n -> get the n-high bits, c -> carry from lower bits
	'R'[n][b]      -> backward, n -> add n to PC
	'A'[n][s|u]    -> align in a paragraph of n-bits
        'L'[n][s|u]    -> get the n-low bits

 There may be more than 1 modifier per built-in base. If H and L are
in the same base, only the first one is taken. 

 The order in which the modifiers appear is NOT relevant.

 s -> null terminated string with the conversion specifier (without the %)

*/
static int is_builtin_marker(char *s) {

  int ret_val = BI_NONE;
  char *st = s;

  /* first check the base */
  if ( *s == 'e' && *(s+1) == 'x' && *(s+2) == 'p' ) {
    ret_val |= BI_BASE_EXP_MASK;
    st += 3;
  }
  else if ( *s == 'a' && *(s+1) == 'd' && *(s+2) == 'd' && *(s+3) == 'r' ) {
    ret_val |= BI_BASE_ADDR_MASK;
    st += 4;
  }
  else if ( *s == 'i' && *(s+1) == 'm' && *(s+2) == 'm') {
    ret_val |= BI_BASE_IMM_MASK;
    st += 3;
  }
  else return BI_NONE;

  /* st points to the beginning of the modifiers now */
  while (*st != '\0') {
   
    switch (*st) {

    case 'L':
      if (ret_val & BI_MD_LO_MASK ||
	  ret_val & BI_MD_HI_MASK) return BI_MD_INVALID_MASK;
      ret_val |= BI_MD_LO_MASK;

      /* check for an integer following */
      while (*(st+1) >= '0' && *(st+1) <= '9') st++;
      if (*(st+1) == 's' || *(st+1) == 'u') st++;

      break;

    case 'H':
      if (ret_val & BI_MD_HI_MASK ||
	  ret_val & BI_MD_LO_MASK) return BI_MD_INVALID_MASK;
      ret_val |= BI_MD_HI_MASK;

      /* check for an integer following */
      while (*(st+1) >= '0' && *(st+1) <= '9') st++;
      if (*(st+1) == 'c') st++;
      if (*(st+1) == 's' || *(st+1) == 'u') st++;

      break;

    case 'A':
      if (ret_val & BI_MD_AL_MASK) return BI_MD_INVALID_MASK;
      ret_val |= BI_MD_AL_MASK;
      /* check for an integer following */
      while (*(st+1) >= '0' && *(st+1) <= '9') st++;
      if (*(st+1) == 's' || *(st+1) == 'u') st++;

      break;

    case 'R':
      if (ret_val & BI_MD_REL_MASK) return BI_MD_INVALID_MASK;
      ret_val |= BI_MD_REL_MASK;

      /* check for an integer following */
      while (*(st+1) >= '0' && *(st+1) <= '9') st++;
      if (*(st+1) == 'b') st++;
      
      break;

    default:
      return BI_MD_INVALID_MASK;
    }

    st++;
  }

  return ret_val;
}




/**********************************************

  ac_asm_map functions

  See the header of this file for basic information.
  Following you will find detailed description of the implementation.

  When a function might indicate an error, it will return 0 and error_msg will
contain the ASCII message of the error.

  The order of the symbols and markers in the lists is the same as they appear
in the source code (the new symbols are added in the end of list, so this will
be true as long as the adding functions are called with the right ordering of
the symbols)

**********************************************/


/*
  It's the first function to call when a mapping block (ac_asm_map) is found in
the source file. It will clean some internal variables (symbol_name_list) and
add one mapping element to mapping_list. The list of symbols (map_symbol_list)
used by the current mapping element is also initialized and ready for eventual
adds through add_mapping_symbol functions.  The symbol names for each
symbol-value pair are stored in the map_symbol_list which is cleared after a
value is assigned to its symbol(s) through add_symbol_value.

  marker -> the name of the mapping block (it's not parsed to check for valid
  characters since the bison parser already gives us a valid string)

*/
int acpp_asm_create_mapping_block(char *marker, char *error_msg)
{
  /* if it's a built-in marker, quit */
  int bi = is_builtin_marker(marker);
  if (bi != BI_NONE) {
    sprintf(error_msg, "Invalid conversion specifier '%s' . It's a built-in type", marker);
    return 0;
  }

  /* if this marker is already used, quit */
  if (find_mapping_marker(marker) != NULL) {
    sprintf(error_msg, "Conversion specifier already used: '%s'", marker);
    return 0;
  }

  /* clear the list of symbols name */
  symbol_name_list = NULL;
  symbol_name_list_tail = NULL;

  /* creates a new ac_map_list element and inserts it in the tail of the list */
  ac_asm_map_list *t_m = (ac_asm_map_list *) malloc(sizeof(ac_asm_map_list));


  t_m->marker = (char *) malloc(strlen(marker)+1);
  strcpy(t_m->marker, marker);
  t_m->used_where = 0;
  t_m->next = NULL;

  if (!mapping_list) {
    mapping_list = t_m;
    mapping_list_tail = mapping_list;
  }
  else {
    mapping_list_tail->next = t_m;
    mapping_list_tail = t_m;
  }

  /* initializes the symbol map list */
  map_symbol_list = &t_m->symbol_list;
  *map_symbol_list = NULL;
  map_symbol_list_tail = NULL;

  return 1;
}



/*
   This adds a single symbol to the current symbol names (symbol_name_list). A
  symbol-value pair is only added to the mapping list when a call to
  add_symbol_value is done.
  

   symbol -> the symbol to add. It must be unique for this mapping block (error
if not). The symbol string is parsed to check for invalid characters (checked
in is_map_symbol_* functions).

*/
int acpp_asm_add_mapping_symbol(char *symbol, char *error_msg) {

  /* if the symbol was already added, quit */
  if (find_mapping_symbol(symbol) != NULL) {
    sprintf(error_msg, "Symbol already used: '%s'", symbol);
    return 0;
  }

  /* if the symbol was not added but is queued, quit as well */
  strlist *t_sl = symbol_name_list;
  while (t_sl != NULL) {
    if (!strcmp(t_sl->str, symbol)) {
      sprintf(error_msg, "Symbol already used: '%s'", symbol);
      return 0;
    }
    t_sl = t_sl->next;
  }

  /* ok, symbol not defined.. validate it now */
  char *s1 = symbol;

  if (!is_map_symbol_begin(s1)) {
    sprintf(error_msg, "Symbol '%s' cannot start with character '%c'", symbol, *s1);
    return 0;
  }

  if (*s1 != '\0') { /* this condition validates empty symbols "" */

    while (1) {
    
      if (*(s1+1) == '\0') {
	if (!is_map_symbol_end(s1)) {
	  sprintf(error_msg, "Symbol '%s' cannot end with character '%c'", symbol, *s1);
	  return 0;
	}
	break;
      }
      
      s1++;
      if (!is_map_symbol_name(s1)) {
	sprintf(error_msg, "Symbol '%s' cannot have character '%c' in its name", symbol, *s1);
	return 0;
      }
      
    }
  }

  /* ok, symbol validated.. so add it to the end of symbol_name_list */

  strlist *t_s = (strlist *) malloc(sizeof(strlist));
  t_s->str = (char *) malloc(strlen(symbol)+1);
  strcpy(t_s->str, symbol);
  t_s->next= NULL;

  if (!symbol_name_list) { 
    symbol_name_list = t_s;
    symbol_name_list_tail = symbol_name_list;
  }
  else {
    symbol_name_list_tail->next = t_s;
    symbol_name_list_tail = t_s;
  }

  return 1;
}


/*
 
  This function adds ranged symbols (those with [..]) to the
symbol_name_list. It does some range checking. Only non-negatives integers are
allowed. It works by building each string with the range specified and calling
add_mapping_symbol() for each single symbol created.

 sb -> string before a range specification - may be NULL
 sa -> string after a range specification - may be NULL
 r1 -> start range value (must be >= 0)
 r2 -> end range value (must be >= 0)

Note: 'sb' and 'sa' cannot be NULL both at the same time. This is treated as an
internal error.

*/
int acpp_asm_add_mapping_symbol_range(char *sb, char *sa, int r1, int r2, char *error_msg) {

  unsigned int range = abs(r2-r1);
  int increment_sign = (r1 < r2) ? 1 : -1;
  char final_symbol[200];  /* a guess :/ */

  if (sb == NULL && sa == NULL) { 
    sprintf(error_msg, "Internal parser error");
    return 0;
  }

  if (r1 < 0 || r2 < 0) {
    sprintf(error_msg, "Only positive values are accepted");
    return 0;
  }
  
  /* creates a string and adds it calling add_mapping_symbol for each symbol created */
  final_symbol[0] = '\0';
  char *p_fs = final_symbol;
  char *saved_p_fs;

  if (sb != NULL) strcpy(p_fs, sb);
  while (*p_fs != '\0') p_fs++;

  saved_p_fs = p_fs;

  int i;
  for (i=0; i<=range; i++) {
    p_fs = saved_p_fs;
  
    sprintf(p_fs, "%d", r1+(i*increment_sign));

    while (*p_fs != '\0') p_fs++;

    if (sa != NULL) strcpy(p_fs, sa);

    if (!acpp_asm_add_mapping_symbol(final_symbol, error_msg)) return 0;

  }

  return 1;
}


/*
  This function should be called after adding symbols through the
add_mapping_symbol functions.  It assigns the value (or a range of values) to
the symbols added. The range can include negative values. If val1 == val2 then
it's a single value and all the symbols in symbol_name_list will be assigned
this value. In case of a range of values, the number of symbols added must
equal the value range or an error will raise.

  val1 -> start range value (any integer)
  val2 -> end range value (any integer)

  Note: Although val1 and val2 might be negative values, the lex doesn't
recognize negative values at the moment of this writing.

*/
// if val1 = val2 then its a unique value; otherwise it's a range
int acpp_asm_add_symbol_value(int val1, int val2, char *error_msg) {

  int range = abs(val2-val1);
  int increment_sign = (val1 < val2) ? 1 : -1;
  int symbol_number;

  /* if it's a ranged symbol, the number of symbol names added must equal the value range */
  if (range) {

    symbol_number = 0;
    strlist *t_sl = symbol_name_list;

    /* counts the number of symbols added */
    while (t_sl != NULL) {
      symbol_number++;
      t_sl = t_sl->next;
    }

    if (range+1 != symbol_number) {
      sprintf(error_msg, "Different ranges specified in symbol mapping");
      return 0;
    }  
  }

  /* assign each symbol to its value and adds to the map_symbol_list list */
  strlist *t_sl = symbol_name_list;
  ac_asm_symbol *t_symbol = NULL;
  
  int i=0;
  while (t_sl != NULL) {

    /* creates the symbol */
    t_symbol = (ac_asm_symbol *) malloc(sizeof(ac_asm_symbol));

    t_symbol->symbol = t_sl->str;
    t_symbol->value = val1;
    if (range) t_symbol->value += (i*increment_sign);
    t_symbol->next = NULL;

    /* insert it in the map_symbol_list */
    if (!*map_symbol_list) { 
      *map_symbol_list = t_symbol;
      map_symbol_list_tail = *map_symbol_list;
    }
    else {
      map_symbol_list_tail->next = t_symbol;
      map_symbol_list_tail = t_symbol;
    }

    i++;
    t_sl = t_sl->next;
  }

  /* done with the name list */
  symbol_name_list = NULL;
  symbol_name_list_tail = NULL;

  // TODO: free symbol_name_list memory

  return 1;
}






/**********************************************

  set_asm functions

  See the header of this file for basic information.
  Following you will find detailed description of the implementation.

  When a function might thrown an error, it will return 0 and error_msg will
contain the ASCII message of the error.

  The list asm_insn_list is in alphabetical ordering. In case of a same
mnemonic name, the order follows the way the insn syntax appear in the source
file.

**********************************************/

/*
  Check if a mapping marker pointed by 's' is valid. 

  str -> string to be validated (after the '%')
  marker -> string in which the marker name will be stored (must be allocated 
  by the caller) (only if returned value == 1)


  str will always point to the next element after the marker name (if there 
  is a ']', it will point after that)
*/
static int validate_marker(char **str, char *marker, char *error_msg) {
  int has_lbrack = 0;  
  char *s = marker;

  *s = '\0';

  if (**str == '[') { 
    has_lbrack = 1;
    (*str)++;
  }

  while (is_identifier(*str)) { /* '-' -> special character used with modifiers */
    *s = **str;
    s++;
    (*str)++;
  }
  
  if (has_lbrack && **str != ']') {
    sprintf(error_msg, "No matching ']' found");
    return 0;
  }
  else if (s == marker) {
    sprintf(error_msg, "No marker specified");
    return 0;
  }


  if (has_lbrack && **str == ']') (*str)++;

  *s = '\0';

  return 1;
}




/*
  When parsing an 'native' insn, this function should be the one called
first. It cleans internal variables and states.

*/
void acpp_asm_new_insn() {

  pseudo_list = NULL;      /* make sure no pseudo-op will be added */
  num_pseudo_insn = 0;

  num_args_found = 0;  
  c_image = 0;

  in_error = 0; /* keep track of errors */

  /* empty strings */
  formatted_asm_str[0] = '\0';
  formatted_arg_str[0] = '\0';
}


/*
  This function validates the asm string in the .ac files and saves a formatted
version internally. The formatting done basicly removes additional whitespaces
and put an additional scape caracter for every one found.  The formatted asm
string is used later when creating the final version (including operands) as
used by the assembler generated by the acasm tool.  Pseudo-ops members insn
uses a lightly different format (following the '%' there is a number, not
identifier) that are stored internally in the formatted_pseudo variable (when
is_pseudo == 1).

  asm_str   -> original string, as found in the .ac files with the set_asm 
               directives

  is_pseudo ->  0 - 'native' instructions. 
              !0 - pseudo-ops members have different semantics for the % arguments.

  Notes: 
       After each %(marker) sequence it's put a ':' character.
       Only the marker (%) character is saved in the mnemonic part of the string. 
       The type of the marker is saved in 'mnemonic_marker'.
       Base built-in types cannot be defined by the user.

  Some examples of outputed format_asm_str:
  
     input  -> "add %reg, %reg"
     output -> "add %reg:,%reg:"

     input  -> "add%ex %reg"
     output -> "add% %reg:"


  TODO:
   falta validar no caso de pseudo_instr que os mnemonicos nao possam conter markers 
   (jah q is_pseudo == 0 e nao tem como saber se eh pseudo ou nao a insn base)
   checar por sintaxe jah definida das instrucoes declaradas sob a pseudo?

*/
int acpp_asm_parse_asm_string(char *asm_str, int is_pseudo, char *error_msg) {

  char *s = asm_str;        /* s     -> pointer to input asm string */
  char *s_out;              /* s_out -> pointer to formatted asm string (module local storage) */

  /*
   In case of a pseudo member insn, we should not clear the numbers of operands
expected for the pseudo-op insn, since it's used to check the limit of
arguments one pseudo member may have.
  */
  if (!is_pseudo) num_args_expected = 0;

  /* choose which buffer to use: native or pseudo insn */
  if (is_pseudo) s_out = formatted_pseudo;
  else s_out = formatted_asm_str;    

  *s_out = '\0';

  mnemonic_marker = NULL;


  /************ mnemonic parsing **************/

  /* eats whitespaces from input */
  while (isspace(*s)) s++;

  if (*s == '\0') {
    sprintf(error_msg, "No assembly syntax specified");
    in_error = 1;
    return 0;
  }

  if ((is_pseudo && !is_pseudo_mnemonic_begin(s)) || !is_mnemonic_begin(s) ) {
    sprintf(error_msg, "Invalid symbol found in the beginning of mnemonic string: '%c'", *s);
    in_error = 1;
    return 0;
  }


  /* The mnemonic is parsed till we have found the first whitespace */
  while (*s != '\0' && !isspace(*s)) {


    if (is_pseudo && !is_pseudo_mnemonic_name(s)) {
	sprintf(error_msg, "Invalid symbol found in mnemonic string: '%c'", *s);
	in_error = 1;
	return 0;      
    }
    else if (!is_mnemonic_name(s)) {
      sprintf(error_msg, "Invalid symbol found in mnemonic string: '%c'", *s);
      in_error = 1;
      return 0;      
    }

    /* mnemonic formatting identifier */
    if (*s == '%' && !is_pseudo) {

      /* only the '%' remains in the string... the type of the marker is saved in mnemonic_marker */

      if (mnemonic_marker != NULL) { /* already has 1 marker attached to this mnemonic */
        sprintf(error_msg, "Maximum number of markers per mnemonic is 1 in this version");
	in_error = 1;
        return 0;
      }	 

      s++;

      if (!validate_marker(&s, s_out, error_msg)) {
	in_error = 1;
	return 0;
      }

      int bi = is_builtin_marker(s_out);

      if (bi != BI_NONE) {
	sprintf(error_msg, "Built-in markers cannot be used with mnemonic");
	in_error = 1;
	return 0;
      }

      ac_asm_map_list *ml = find_mapping_marker(s_out);
      if (ml == NULL) {
	sprintf(error_msg, "Invalid conversion specifier: '%%%%%s'", s_out);  
	in_error = 1;
	return 0;
      }

      *s_out = '%';
      s_out++; 

      mnemonic_marker = ml;
      ml->used_where |= 2;
      num_args_expected++;

    }
    else if (*s == '\\') {  

      /* only valid scape sequence is \% */

      /* saves only one '\' 'cos it will be removed from the final mnemonic string anyway */
      *s_out = '\\';
      s_out++;
      s++;

      if (*s != '%') {
	sprintf(error_msg, "Invalid scape sequence found: '%c'", *s);
	in_error = 1;
	return 0;
      }
      
      *s_out = '%';
      s_out++;
      s++;

    }
    else {	 
      *s_out = *s;
      s_out++;
      s++;
    }
  }
 

  /************ operands parsing **************/

  while (isspace(*s)) s++;

  if (*s == '\0') { /* no operand found */
    *s_out = '\0';
    return 1;   
  }

  *s_out = ' ';
  s_out++;

  while (*s != '\0') {

    while (isspace(*s)) s++;
    
    if (!is_operand_string(s)) {
      sprintf(error_msg, "Invalid character found in assembly string: '/%c'", *s);
      in_error = 1;
      return 0;
    }

    if (is_group_token(s)) {

      while (is_group_token(s)) {
	*s_out = *s;
	s_out++;
	s++;
      }

      /* eats all spaces; leaves one if next token is another group token */
      if (isspace(*s)) {
	while (isspace(*s)) s++;

	if (is_group_token(s)) {
	  *s_out = ' ';
	  s_out++;
	}
      }
      
      continue;
    }

    if (*s == '%') {
      *s_out = *s;
      s_out++;
      s++; 

      if (is_pseudo) {
        if (*s >= '0' && *s <= '9') {

	  if (*s > ('0' + num_args_expected-1)) {
	    sprintf(error_msg, "Argument in macro not valid: '%c'", *s);
	    in_error = 1;
	    return 0;
	  }

	  *s_out = *s;
          s_out++;
	  s++;
	}
	else {
          sprintf(error_msg, "Invalid macro substitution: '%c'", *s);
	  in_error = 1;
          return 0;
	}
      }
      else {

	if (!validate_marker(&s, s_out, error_msg)) {
	  in_error = 1;
	  return 0;	
	}

	/* first check for a built-in marker */
	int bi = is_builtin_marker(s_out);
	if (bi != BI_NONE && (bi & BI_MD_INVALID_MASK)) {
	  sprintf(error_msg, "Invalid modifier used with built-in conversion specifier");
	  //	  printf("'%s' \n", s_out);
	  in_error = 1;
	  return 0;
	}
	else if (bi == BI_NONE) { /* no builtin, check for a user-defined marker one */

	  ac_asm_map_list *ml = find_mapping_marker(s_out);
	  if (ml == NULL) {
	    /* the following sequence of %'s are saved as %% and then to % when displaying.
	       Do not try to optimize it ^^ */
	    sprintf(error_msg, "Invalid conversion specifier: '%%%%%s'", s_out);

	    in_error = 1;
	    return 0;
	  }
	  ml->used_where |= 1;
	}

	num_args_expected++;
	while (*s_out != '\0') s_out++;
	*s_out = ':';
	s_out++;	  
	
      }
    } 
    else if (*s == '\\') {
      /* only valid scape sequence is \% */

      /* saves two '\' 'cos this string will be used as a structure in the asm generated
         the first '\' will scape the second '\', so we'll have a single '\%' in the end */
      *s_out = '\\';
      s_out++;
      *s_out = '\\';
      s_out++;
      s++;

      if (*s != '%') {
	sprintf(error_msg, "Invalid scape sequence found: '%c'", *s);
	in_error = 1;
	return 0;
      }
      
      *s_out = '%';
      s_out++;
      s++;
    }
    else { 
      *s_out = *s;
      s_out++;
      s++;    
    }
  }

  *s_out = '\0';

  return 1;
}



/*

 Parses an argument relative to one of the markers defined in the syntax
string. An argument is basicly a field of the instruction where the values will
be encoded by the assembler generated.  It validates the field (check if it
exists in the insn format) and create an internal string representation
(formatted_arg_str) which is used by the assembler generated by the acasm
tool. The format of this string is a number describing the field within the
format (from the left to the right of the insn format) followed by ':'.


 pf  -> pointer to the format associated with this insn field
 field_str -> an argument string as found in the ArchC source file 
 (for constant arguments, use const version of this function)

 Note:
   For mnemonics arguments, 'formatted_arg_str' is not changed.
 
 example:

  input: "imm"  (let's say it's field 3)
  output: "3:"  (stored internally and used when creating the final asm string)

*/
int acpp_asm_parse_asm_argument(ac_dec_format *pf, char *field_str, char *error_msg) {

  static char *f_arg_str_p;

  /* gets the field ID - from left to the right of the format */
  ac_dec_field *pfield = pf->fields;
  int f_id=0;
  for(; pfield != NULL && strcmp(field_str, pfield->name); pfield = pfield->next) f_id++;
  if( pfield == NULL ) {
    sprintf(error_msg, "Invalid field used as argument: '%s'", field_str);
    in_error = 1;
    return 0;
  }  

  /* whenever we start building a new formatted_arg_str, make a pointer to its beggining */
  if (num_args_found == 0)
    f_arg_str_p = formatted_arg_str;

  num_args_found++;

  /* if it's a mnemonic argument (always and only the first argument can be) then saves the
     field in 'mnemonic_marker_field. No internal string is saved */
  if (mnemonic_marker != NULL && num_args_found == 1) {
     mnemonic_marker_field = pfield;
     return 1;
  }

  //  sprintf(&(formatted_arg_str[arg_str_ind]), "%d:", f_id);
  sprintf(f_arg_str_p, "%d:", f_id);

  /* move the pointer to the end of the string */
  while (*f_arg_str_p != ':') f_arg_str_p++;

  f_arg_str_p++;
  *f_arg_str_p = '\0';

  return 1;
}


/*
  Handles parsing of set_asm constant arguments. A constant argument may have
  two forms:
    . an int const of the form:   <field> = int_value
    . a string const of the form: <field> = str_value

  pf        -> pointer to the format of the instruction this argument is being 
               set to
  field_str -> <field> field of the instruction format which will be constant
  iconst_field, sconst_field -> integer and string const field. Only one must be 
               valid at a time. A value of NULL in sconst_field indicates this 
               constant argument is of type int. Otherwise it's a string type
 
  Constant argument does not alter the 'formatted_arg_str'. Instead, it uses
c_image to store the const value assigned by the user in an ArchC source file.

  examples:

   ArchC source:  "imm = 10"
      . 10 will be encoded in its right position within the format field 'imm' 
        and saved in 'c_image'

   ArchC source:  "imm = $ra"
      . first the value of '$ra' will be found, then it'll encoded and saved 
        in 'c_image'

*/
int acpp_asm_parse_const_asm_argument(ac_dec_format *pf, char *field_str, int iconst_field, char *sconst_field, char *error_msg) {

  /* if str const type, find the symbol value */
  if (sconst_field != NULL) {  
    long int symbol_value;

    if (!get_symbol_value(sconst_field, &symbol_value)) {
      sprintf(error_msg, "Invalid symbol assigned in argument declaration: '%s'", sconst_field);
      in_error = 1;
      return 0;
    }

    iconst_field = symbol_value;
  }

  /* find the field */
  ac_dec_field *pfield = pf->fields;
  for(; pfield != NULL && strcmp(field_str, pfield->name); pfield = pfield->next);
  if( pfield == NULL ) {
    sprintf(error_msg, "Invalid field used as argument: '%s'", field_str);
    in_error = 1;
    return 0;
  }  

  /* encode it, saving in 'c_image' 
     note that there will be only 1 encoding for each format field, so it's ok to use the OR operation */
  c_image |= ac_asm_encode_insn_field(iconst_field, pf, pfield);

  return 1;
}



/*
  This should be called after the syntax string and argument field are
  processed.  
  Strings 'formatted_asm_str' and 'formatted_arg_str' are merged
  and a new acpp_asm_insn is inserted in the insn list. In case a mnemonic
  marker exists, its expansion is done here also.

  p -> pointer to the relative ac_dec_insn 


  Format of insn->operand :
              after a marker -> ':' <integer> ':'   - among markers, there may be literals


  example:
    asm_str = "%reg:,%imm:"
    arg_str = "2:1:"

    insn->operand = "%reg:2:%imm:1:"

*/
int acpp_asm_end_insn(ac_dec_instr *p, char *error_msg)
{
  if (num_args_found != num_args_expected) {
    sprintf(error_msg, "Invalid number of arguments");
    return 0;
  }

  /* An error already happened... don't waste more time then */
  if (in_error) return 1;

  /* get the begin of the operand string */
  char *p_bos = formatted_asm_str;

  while (*p_bos != '\0' && !isspace(*p_bos)) p_bos++;
  
  if (isspace(*p_bos)) {
    *p_bos = '\0';  /* formatted_asm_str points to whole mnemonic string now */
    p_bos++;
  }


  /********** 
   Creates the operand string (merge operand part of formatted_asm_str with formatted_arg_str)
  ***********/

  char *p_fas = formatted_arg_str;
  /* operand_string is the final operand string saved in insn->operand */
  char *operand_string = (char *) malloc(strlen(p_bos)+strlen(p_fas)+1);
  char *p_os = operand_string;
  char *oper = p_bos;

  /* merging loop */
  while (*oper != '\0') {
  
    if (*oper == '\\') { /* escaping '\%' */
      *p_os = *oper;
      p_os++;
      oper++;
      if (*oper == '%') {
        *p_os = *oper;
	p_os++;
        oper++;
      }      
    }
    /* a marker. get the relative arg and merge */
    else if (*oper == '%') {   
      *p_os = *oper;
      p_os++;
      oper++;

      /* pass through the marker name */
      while (*oper != ':') {
        *p_os = *oper;
	p_os++;
	oper++;      
      }
      *p_os = *oper;
      p_os++;
      oper++;

      /* concatenate arg_str */
      do {   
        *p_os = *p_fas;
        p_os++;
        p_fas++;
      } while ((*p_fas != ':'));

      *p_os = *p_fas;
      p_os++;      
      p_fas++;
    }       
    else {
      *p_os = *oper;
      p_os++;
      oper++;
    }
  }
  *p_os = '\0';  


  /********** 
   Creates the mnemonic (expand it if needed, and creates the ac_asm_insn)
  ***********/

  ac_asm_symbol *sl = NULL;

  if (mnemonic_marker != NULL)
    sl = mnemonic_marker->symbol_list;

  int exit_loop = 0;
  while (!exit_loop) {

    /* creates a new ac_asm_insn for each mnemonic */

    ac_asm_insn *insn = (ac_asm_insn *) malloc(sizeof(ac_asm_insn));
    /* note that if there's more than 1 mnemonic, operand_string memory will be shared
       among all insn created - take care when freeing this memory :S */
    insn->operand = operand_string;
    insn->insn = p;
    insn->pseudolist = pseudo_list;
    insn->num_pseudo = num_pseudo_insn;
    insn->const_image = c_image;
    insn->next = NULL;

    /* if there's a mnemonic marker, expand it */  
    if (mnemonic_marker != NULL)
    {
      if (sl == NULL) break;   /* last symbol, get out */

      /* -1 because of the '%' we are deleting */
      insn->mnemonic = (char *) malloc(strlen(formatted_asm_str)+strlen(sl->symbol)+1-1); 
      char *mnem = insn->mnemonic;
      char *mt = formatted_asm_str;

      /* find the first %, where we must concatenate the symbol */
      while (*mt != '%') {

	/* we don't need to save scape character in the mnemonic string, since the only
	   valid scape sequence is '%'. Note that the memory allocated to insn->mnemonic
	   may not be complety used because of this.
	 */
	if (*mt == '\\') 
	  mt++; 

        *mnem = *mt;
	mnem++;
        mt++;
      }
      mt++;

      /* concatenate symbol name */
      strcpy(mnem, sl->symbol);
      mnem += strlen(sl->symbol);

      /* concatenate the rest of the string */
      while (*mt != '\0') {

	if (*mt == '\\') 
	  mt++;
	
        *mnem = *mt;
	mnem++;
	mt++;
      }
      *mnem = '\0';

      /* now encode the information relative to the mnemonic */
      ac_dec_format *pf = format_ins_list;

      /* find the format */
      for(; pf != NULL && strcmp(insn->insn->format, pf->name); pf = pf->next);
      if( pf == NULL ) {
	sprintf(error_msg, "Internal Error"); /* this should never happen */
	return 0;
      }  
      
      /* encode it */
      insn->const_image |= ac_asm_encode_insn_field(sl->value, pf, mnemonic_marker_field);

      sl = sl->next;      

    }
    else { /* only one mnemonic */

      insn->mnemonic = (char *) malloc(strlen(formatted_asm_str)+1);
      strcpy(insn->mnemonic, formatted_asm_str);
 
      exit_loop = 1;
    }


    /* Insert the new ac_asm_insn into the asm_insn_list 
       It's inserted in alphabetical ordering. In case there are two or more
       insns with the same mnemonic, its order in the list is the same as they
       appear in the ArchC source file */

    if (asm_insn_list == NULL) {
      asm_insn_list = insn;
    } else {

      ac_asm_insn *t = asm_insn_list;
      ac_asm_insn *last = asm_insn_list;
      
      if (strcmp(insn->mnemonic, t->mnemonic) < 0) { /* insert in the head */
        insn->next = asm_insn_list;
        asm_insn_list = insn;
      }
      else {
        t = t->next;
        /* keep the order in the source file */
        while ((t != NULL) && (strcmp(insn->mnemonic, t->mnemonic) >= 0)) {
          t = t->next;
	  last = last->next;
        }
        insn->next = t;
        last->next = insn;
      }
    }
  }

  return 1;
}




/**********************************************

  pseudo_asm functions

  See the header of this file for basic information.
  Following you will find detailed description of the implementation.

  When a function might thrown an error, it will return 0 and error_msg will
contain the ASCII message of the error.

**********************************************/



/*
 This function should be called right after acpp_asm_parse_asm_insn() to
initialize internal variables.
 It creates a dummy argument string buffer (formatted_arg_str) since
pseudo_instrs don't have arguments. This way, acpp_asm_end_insn() can also be
used by pseudo_instrs insn to insert them in the main insn list.

*/
void acpp_asm_new_pseudo() {

  pseudo_list = NULL;
  num_pseudo_insn = 0;
  c_image = 0;
  num_args_found = num_args_expected;

  /* creates a dummy buffer for arg string: '0:' n times */
  int i;
  for (i=0; i<num_args_expected; i++) {
    formatted_arg_str[i*2] = '0';
    formatted_arg_str[i*2+1] = ':';
  }
  formatted_arg_str[num_args_expected*2] = '\0';
}



/*
  This function validates de string 'pseudo' (by callign
acpp_asm_parse_asm_strin with pseudo_field set) and then inserts the formatted
string in the pseudo_list variable at the tail of the list.

  pseudo -> a pseudo string member of a pseudo_instr declaration as found in
  the ArchC source files.

*/
int acpp_asm_add_pseudo_member(char *pseudo, char *error_msg)
{
  strlist *sl = (strlist *)malloc(sizeof(strlist));
  sl->next = NULL;

  if (!acpp_asm_parse_asm_string(pseudo, 1, error_msg)) 
    return 0;

  sl->str = (char *) malloc(strlen(formatted_pseudo)+1);
  strcpy(sl->str, formatted_pseudo);
 
  /* Insert the pseudo string in the list - at the tail */
  strlist *pL = pseudo_list;

  if (pseudo_list == NULL)
    pseudo_list = sl;
  else {
    while (pL->next != NULL) pL = pL->next;
    pL->next = sl;
  }

  num_pseudo_insn++;

  return 1;
}
