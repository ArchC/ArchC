/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      asm_actions.c
 * @author    Alexandro Baldassin
 *            Rafael Auler
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Fri, 02 Jun 2006 10:59:18 -0300
 *
 * @brief     ArchC assembly-related semantic actions
 *
 * @attention Copyright (C) 2002-2008 --- The ArchC Team
 *
 */

/*
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

  Info:



*******************************
 -> set_asm
*******************************

 The general format of a set_asm declaration is:

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
       it tells to expand that in the symbols defined under the format
       identifier (only identifiers created through 'ac_asm_map' are valid);
        example:
          ac_asm_map annul {
      "" = 0;
            ",a" = 1;
          }

          set_asm("bvs%[anul] %expRW", an, ...);
            the generation tool will expand it in:
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


  asm Info:



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


  asm Info:


Some conventions:

- asm specific parser structures use the prefix string 'ac_asm'
  (that's because the decoder structures use 'ac_dec')

- there are no global variables shared among modules. Interface functions are
  used instead. Functions which are used by the parser use the prefix 'acpp_asm'
  whereas those used by the generating tool or this module itself use the prefix
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
#include "asm_actions.h"


/* 
  acasm special declarations variables
*/
static char *comment_chars = NULL;
static char *line_comment_chars = NULL;

/*
  ac_asm_map variables
*/

static ac_asm_map_list *mapping_list = NULL;       /* map list header pointer */
static ac_asm_map_list *mapping_list_tail = NULL;  /* tail pointer */

static char *current_map = NULL; /* current map being parsed*/

/* each mapping element has a collection of symbol-value mapping.
this variable holds the list of the symbol-value mapping of all mapping elements
*/
static ac_asm_symbol *symbol_list = NULL;

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

/* Used when a backend needs the correct ordering of set_asm constructs as it appears in 
   the ArchC source file. */
static ac_asm_insn *asm_insn_list_original_order = NULL;

/* when const fields are used, this variable will store their encoding for the current insn */
static long int c_image;

static ac_asm_insn *current_insn;


/*
  pseudo_instr variables
*/
static long num_pseudo_insn = 0;      /* number of insns a pseudo-op is made up */
static strlist *pseudo_list = NULL;   /* the pseudo list for the current pseudo declaration */



/* variables taken from the bison file */
extern ac_dec_format *format_ins_list;   /* list of formats declared */


/*
 interface functions
*/

ac_asm_insn* ac_asm_get_asm_insn_list() { return asm_insn_list; }
ac_asm_insn* ac_asm_get_asm_insn_list_original_order() { return asm_insn_list_original_order; }
ac_asm_symbol* ac_asm_get_mapping_list() { return symbol_list; }
char * ac_asm_get_comment_chars() 
{
  if (comment_chars != NULL)
    return comment_chars;
  return "#!";
}
char * ac_asm_get_line_comment_chars() 
{
  if (line_comment_chars != NULL)
    return line_comment_chars;
  return "#";
}


/*
 Internal Helper functions
*/


/*
  This looks for the first occurence of 'symbol' in the mapping list.

  This function returns only the first occurence as it's found in the list,
  matching the given map name

  map parameter can be null, in which case the search will look
  in all maps
*/
static int get_symbol_value(char *symbol, char *map, long int *symbol_id) {

  long int symbol_value = -1;  

  int found = 0;
  ac_asm_symbol *sl = symbol_list;
  
  while (sl != NULL) {
    //    if (map == NULL) // if map name is not present, search all maps
    //{
    //if (!strcmp(sl->symbol, symbol)) {
    //  symbol_value = sl->value;
    //  found = 1;
    //  break;
    //}
    //}
    //else {
    if ((!strcmp(sl->symbol, symbol))&&( map == NULL
					 || (!strcmp(sl->map_marker, map)))) {
      symbol_value = sl->value;
      found = 1;
      break;
    }
      //}
    /* remember symbol list now is alphabetically sorted */
    if (strcmp(sl->symbol, symbol)>0)
      break;
    sl = sl->next;
  }

  *symbol_id = symbol_value;

  return found;
}



/*
  Searches for the marker string 'marker' in the mapping_list.
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

  ac_asm_symbol *t_sl = symbol_list;

  while (t_sl != NULL) {
    if ((!strcmp(t_sl->symbol, symbol))&&(!strcmp(t_sl->map_marker, mapping_list_tail->marker)))
    {
      return t_sl;
    }
    /* remember symbol list is alphabetically sorted */
    if (strcmp(t_sl->symbol, symbol)>0)
      break;
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
inline static int is_map_symbol_name(char *s) 
{
/*
  if (isspace(*s) || *s == ';' || *s == '\''
      || (*s == '/' && *(s+1) == '*')
      || (*s == '*' && *(s+1) == '/') )
    return 0;
*/
  if ( (*s >= 'a' && *s <= 'z') ||
       (*s >= 'A' && *s <= 'Z') ||
       (*s >= '0' && *s <= '9') ||
        *s == '.' || *s == '_' )
    return 1;

  return 0;
}

/* empty symbols ("") are valid! */
inline static int is_map_symbol_begin(char *s) 
{
  if ( *s == '\0' || is_map_symbol_name(s) ||
       *s == '%' || *s == '!' ||
       *s == '@' || *s == '#' ||
       *s == '$' || *s == '&' ||
       *s == '*' || *s == '-' ||
       *s == '+' || *s == '=' ||
       *s == '|' || *s == ':' ||
       *s == '<' || *s == '>' ||
       *s == '^' || *s == '~' ||
       *s == '?' || *s == '/' ||
       *s == ',' || *s == '_')
    return 1;

  return 0;
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



static int is_builtin_marker(char *s)
{
  if (!strcmp(s, "exp") || !strcmp(s, "addr") || !strcmp(s, "imm"))
    return 1;
  else return 0;
}

static int create_operand(char **os, ac_operand_list **oper, int mne_mode, char *error_msg)
{
  char **st = os;
  char *stp;
  char savechar;
  int has_lbrack = 0;
  int has_listmod = 0;

  if (**st == '[') {
    has_lbrack = 1;
    (*st)++;
  }

  *oper = (ac_operand_list *)malloc(sizeof(ac_operand_list));
  (*oper)->next = NULL;
  (*oper)->fields = NULL;

  (*oper)->reloc_id = 0;
  (*oper)->oper_id = -1;


  stp = *st;

  /* advances till '(' or a non-identifier symbol */
  while ( (**st != '(') &&
	  is_identifier(*st) )
    (*st)++;

  if (stp == *st) {
    sprintf(error_msg, "No operand specified");
    return 0;
  }

  savechar = **st;
  **st = '\0';

  if (!strcmp(stp, "exp")) 
    {
      if (mne_mode){
        sprintf(error_msg, "Cannot define a mnemonic suffix as type \"exp\"");
        return 0;
      }
      (*oper)->type = op_exp;
    }
  else if (!strcmp(stp, "imm"))
    {
      if (mne_mode){
        sprintf(error_msg, "Cannot define a mnemonic suffix as type \"imm\"");
        return 0;
      }
      (*oper)->type = op_imm;
    }
  else if (!strcmp(stp, "addr"))
    {
      if (mne_mode){
        sprintf(error_msg, "Cannot define a mnemonic suffix as type \"addr\"");
        return 0;
      }
      (*oper)->type = op_addr;
    }
  else {
    ac_asm_map_list *ml = find_mapping_marker(stp);
    if (ml == NULL) {
      sprintf(error_msg, "Invalid conversion specifier: '%%%%%s'", stp);
      return 0;
    }
    (*oper)->type = op_userdef;
  }

  (*oper)->str = (char *)malloc(strlen(stp)+1);
  strcpy((*oper)->str, stp);

  **st = savechar;

/* LISTMODIFIER */
  if ((*st)[0] == '.' && (*st)[1] == '.' && (*st)[2] == '.')
  {
    has_listmod = 1;
    (*st) += 3;
    if ((*oper)->type == op_addr) {
      sprintf(error_msg, "List operator cannot work with \"addr\" operands");
      return 0;
    }
  }
  

/* MODIFIER */

  (*oper)->modifier.name = NULL;
  (*oper)->modifier.type = -1;
  (*oper)->modifier.addend = 0;
  (*oper)->is_list = has_listmod;


  //while (*st == ' ') st++;
  if (**st != '(') {
    if (has_listmod){
      sprintf(error_msg, "List operator \"...\" must implement a modifier");
      return 0;
    }

    if (has_lbrack && **st != ']') {
      sprintf(error_msg, "No matching ']' found");
      return 0;
    }
    if (has_lbrack) (*st)++;
    return 1;
  }

  (*st)++;
  stp = *st;
 
  /* identifier expected */
  while ( (**st >= 'a' && **st <= 'z') ||
          (**st >= 'A' && **st <= 'Z') || 
          (**st >= '0' && **st <= '9') || 
          (**st == '_') ) 
    (*st)++;


  if (stp == *st) {  /* no identifier */
    sprintf(error_msg, "No modifier identifier specified");
    return 0;
  }

  savechar = **st;
  **st = '\0';

  (*oper)->modifier.name = (char *) malloc(strlen(stp)+1);
  strcpy((*oper)->modifier.name, stp);

  **st = savechar;

  while (**st == ' ') (*st)++;

  /* an optional comma + ADDEND */
  if (**st == ')') { 
    (*st)++;
    if (has_lbrack && **st != ']') {
      sprintf(error_msg, "No matching ']' found");
      return 0;
    }
    if (has_lbrack) (*st)++;
     
    return 1;
  }
  if (**st != ',') {
    sprintf(error_msg, "Comma expected but not found");
    return 0;
  }

  (*st)++;

  while (**st == ' ') (*st)++;
  stp = *st;

  while (**st >= '0' && **st <= '9') (*st)++;

  if (stp == *st) {
    sprintf(error_msg, "No addend specified");
    return 0;
  }

  savechar = **st;
  **st = '\0';

  (*oper)->modifier.addend = atoi(stp);

  **st = savechar;

  while (**st == ' ') (*st)++;
  if (**st != ')') { 
    sprintf(error_msg, "No closing ')' found");
    return 0;
  }
  (*st)++;

  if (has_lbrack && **st != ']') {
    sprintf(error_msg, "No matching ']' found");
    return 0;
  }
  if (has_lbrack) (*st)++;

  return 1;
}

/**********************************************
  Special acasm declarations functions
**********************************************/
int acpp_set_assembler_comment_chars(char *input, char *error_msg)
{
  if (comment_chars != NULL)
  {
    sprintf(error_msg, "Assembler comment characters have duplicated declaration: '%s'", input);
    return 0;
  }
  comment_chars = strdup(input);
  return 1;
}

int acpp_set_assembler_line_comment_chars(char *input, char *error_msg)
{
  if (line_comment_chars != NULL)
  {
    sprintf(error_msg, "Assembler line comment characters have duplicated declaration: '%s'", input);
    return 0;
  }
  line_comment_chars = strdup(input);
  return 1;
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
  if (is_builtin_marker(marker)) {
    sprintf(error_msg, "Invalid conversion specifier '%s' . It's a built-in type", marker);
    return 0;
  }

  /* if this marker is already used, quit */
  if (find_mapping_marker(marker) != NULL) {
    sprintf(error_msg, "Conversion specifier already used: '%s'", marker);
    return 0;
  }

  /* creates a new ac_map_list element and inserts it in the tail of the list */
  ac_asm_map_list *t_m = (ac_asm_map_list *) malloc(sizeof(ac_asm_map_list));

  t_m->marker = (char *) malloc(strlen(marker)+1);
  strcpy(t_m->marker, marker);
  t_m->next = NULL;

  if (!mapping_list) {
    mapping_list = t_m;
    mapping_list_tail = mapping_list;
  }
  else {
    mapping_list_tail->next = t_m;
    mapping_list_tail = t_m;
  }

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

/* auxiliary recursive function for linked list freeing */
void str_list_free (strlist *value)
{
  if (value == NULL)
    return;
  str_list_free(value->next);
  free(value->str);
  return;
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

    t_symbol->symbol = strdup(t_sl->str);
    t_symbol->value = val1;
    t_symbol->map_marker = strdup(mapping_list_tail->marker);

    if (range) t_symbol->value += (i*increment_sign);
    t_symbol->next = NULL;

    /* insert it in the symbol_list */
    if (!symbol_list) {
      symbol_list = t_symbol; 
    }
    else {
      ac_asm_symbol *t = symbol_list;
      if (strcmp(t_symbol->symbol, t->symbol) < 0)
      {
        t_symbol->next = symbol_list;
        symbol_list = t_symbol;
      } else
      {
	while ((t->next != NULL) && (strcmp(t_symbol->symbol, t->next->symbol) >= 0)) {
          t = t->next;
        }
        t_symbol->next = t->next;
        t->next = t_symbol;
      }
    }

    i++;
    t_sl = t_sl->next;
  }
  
  /* freeing symbol_name_list memory */
  str_list_free(symbol_name_list);  

  /* done with the name list */
  symbol_name_list = NULL;
  symbol_name_list_tail = NULL;

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

  current_insn = (ac_asm_insn *) malloc(sizeof(ac_asm_insn));
  current_insn->operands = NULL;
  current_insn->op_literal = NULL;
  current_insn->op_literal_unformatted = NULL;
  current_insn->const_fields = NULL;
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
  int mne_mode = 1;         /* mne_mode = 1 if parsing mnemonic suffixes (only userdef symbols, no whitespaces)
                               mne_mode = 0 if parsing ordinary operands
                               mne_mode is set to 0 when the first whitespace is found (mnemonic - operand delimiter) */

  

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


  /* The mnemonic is parsed till we have found the first whitespace or an identifier (continues with operand parsing) */
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

    /* mnemonic suffix (finalize and transfer to operand parsing) */
    if (*s == '%' && !is_pseudo) {
       break;
    }
    else if (*s == '\\') {

      /* only valid escape sequence is \% */

      /* saves only one '\' 'cos it will be removed from the final mnemonic string anyway */
      *s_out = '\\';
      s_out++;
      s++;

      if (*s != '%') {
        sprintf(error_msg, "Invalid escape sequence found: '%c'", *s);
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

  if (!is_pseudo) {
    *s_out = '\0';
    current_insn->mnemonic = (char *)malloc(strlen(formatted_asm_str)+1);
    strcpy(current_insn->mnemonic, formatted_asm_str);

    s_out = formatted_asm_str;

    current_insn->operands = NULL;
  }

  /************ operands parsing **************/

  ac_operand_list *operands = NULL;

  /* (1) if it isn't a mnemonic suffix, appends a whitespace to the output and
     ignores subsequent whitespaces */ 
  if (isspace(*s)) {
    *s_out = ' ';
    mne_mode = 0; /* now parsing operands instead of mnemonic suffixes */
    s_out++;
    s++;
    while (isspace(*s)) s++;
  }

  if (*s == '\0') { /* no operand found */
    if (!mne_mode)
      s_out--; /* Ignores the previously appended whitespace */
    *s_out = '\0';

    if (!is_pseudo) {
      current_insn->op_literal = (char *)malloc(strlen(formatted_asm_str)+1);
      strcpy(current_insn->op_literal, formatted_asm_str);
      current_insn->op_literal_unformatted = (char *)malloc(strlen(asm_str)+1);
      strcpy(current_insn->op_literal_unformatted, asm_str);
    }
    return 1;
  }

  while (*s != '\0') {
    /* Checks for whitespaces, ending mnemonic suffixes parsing mode */
    if (mne_mode && isspace(*s)) { /* First whitespace found */
       *s_out = ' ';
       mne_mode = 0; /* Now parsing operands instead of mnemonic suffixes */
       s_out++;
       s++;
    }
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
      if (isspace(*s) && !mne_mode) {
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

        ac_operand_list *operand = NULL;
        if (!create_operand(&s, &operand, mne_mode, error_msg)) {
          in_error = 1;
          return 0;
        }

        num_args_expected++;

        if (operands == NULL) {
           operands = operand;
           operands->next = NULL;
        }
        else {
           ac_operand_list *head = operands;
           while (head->next != NULL) head = head->next;
           head->next = operand;
           operand->next = NULL;
        }
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

  if (!is_pseudo) {
    current_insn->op_literal = (char *)malloc(strlen(formatted_asm_str)+1);
    strcpy(current_insn->op_literal, formatted_asm_str);
    current_insn->op_literal_unformatted = (char *)malloc(strlen(asm_str)+1);
    strcpy(current_insn->op_literal_unformatted, asm_str);
    current_insn->operands = operands;
  }

  return 1;
}



/*

 Parses an argument relative to one of the markers defined in the syntax
string. An argument is basicly a field of the instruction where the values will
be encoded by the assembler generated.  It validates the field (check if it
exists in the insn format) and create an internal string representation
(formatted_arg_str) which is used by the assembler generated by the acasm
tool. The format of this string is a number describing the field within the
format (from left to right of the insn format) followed by ':'.


 pf  -> pointer to the format associated with this insn field
 field_str -> an argument string as found in the ArchC source file
 (for constant arguments, use const version of this function)


 example:

  input: "imm"  (let's say it's field 3)
  output: "3:"  (stored internally and used when creating the final asm string)

*/
int acpp_asm_parse_asm_argument(ac_dec_format *pf, char *field_str, int is_concatenated, char *error_msg) {

  static char *f_arg_str_p;

  if (pf == NULL) {
    sprintf(error_msg, "Ignoring undeclared instruction set_asm argument");
    return 0;
  }
  /* gets the field ID - from left to the right of the format */
  ac_dec_field *pfield = pf->fields;
  int f_id=0;
  for(; pfield != NULL && strcmp(field_str, pfield->name); pfield = pfield->next) f_id++;
  if( pfield == NULL ) {
    sprintf(error_msg, "Invalid field used as argument: '%s'", field_str);
    in_error = 1;
    return 0;
  }


  /* whenever we start building a new formatted_arg_str, make a pointer to its beginning */
//  if (num_args_found == 0)
//    f_arg_str_p = formatted_arg_str;

  if (!is_concatenated)
    num_args_found++;

  ac_asm_insn_field *newfield = (ac_asm_insn_field *)malloc(sizeof(ac_asm_insn_field));

  newfield->name = (char *)malloc(strlen(pfield->name)+1);
  strcpy(newfield->name, pfield->name);

  newfield->size = pfield->size;
  newfield->first_bit = pfield->first_bit;
  newfield->id = f_id;
//  newfield->reloc_id = 0;
  newfield->next = NULL;

  unsigned counter = num_args_found - 1;
  ac_operand_list *matching_op = current_insn->operands;

  if (matching_op == NULL) {
    sprintf(error_msg, "Invalid number of arguments");
    in_error = 1;
    return 0;
  }

  while (counter) {
    matching_op = matching_op->next;
    if (matching_op == NULL) {
      sprintf(error_msg, "Invalid number of arguments");
      in_error = 1;
      return 0;
    }

    counter--;
  }


  if (matching_op->fields == NULL) {
    matching_op->fields = newfield;
    matching_op->fields->next = NULL;
  }
  else {
    ac_asm_insn_field *head = matching_op->fields;
    while (head->next != NULL) head = head->next;
    head->next = newfield;
  }


  //  sprintf(&(formatted_arg_str[arg_str_ind]), "%d:", f_id);
//  sprintf(f_arg_str_p, "%d:", f_id);

  /* move the pointer to the end of the string */
//  while (*f_arg_str_p != ':') f_arg_str_p++;

//  f_arg_str_p++;
//  *f_arg_str_p = '\0';

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
  map -> identifies the <ac_asm_map> containing the sconst_field to be translated
  iconst_field, sconst_field -> integer and string const field. Only one must be
               valid at a time. A value of NULL in sconst_field indicates this
               constant argument is of type int. Otherwise it's a string type

  Constant argument does not alter the 'formatted_arg_str'. Instead, it uses
c_image to store the const value assigned by the user in an ArchC source file.

  examples:

   ArchC source:  "imm = 10"
      . 10 will be encoded in its right position within the format field 'imm'
        and saved in 'c_image'

   ArchC source:  "imm = reg.map_to("$ra")"
      . first the value of '$ra' will be found in map "reg", then it'll encoded and saved
        in 'c_image'

*/
int acpp_asm_parse_const_asm_argument(ac_dec_format *pf, char *field_str, char *map, int iconst_field, char *sconst_field, char *error_msg)
{

  /* if str const type, find the symbol value */
  if (sconst_field != NULL) {
    long int symbol_value;
    if (map != NULL) { // parsing constructions like "mapname.map_to("valname")"
      if (find_mapping_marker(map) == NULL) {
	sprintf(error_msg, "Undefined map name '%s' in argument declaration: '%s.map_to(\"%s\")'", map, map, sconst_field);
      }
      if (!get_symbol_value(sconst_field, map, &symbol_value)) {
	sprintf(error_msg, "Invalid map symbol assigned in argument declaration: '%s.%s'", map, sconst_field);
	in_error = 1;
	return 0;
      }

    } else { // parsing constructions like ""valname""
      if (!get_symbol_value(sconst_field, NULL, &symbol_value)) {
	sprintf(error_msg, "Invalid map symbol assigned in argument declaration: '%s'", sconst_field);
	in_error = 1;
	return 0;
      }
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

  ac_const_field_list *cfield = (ac_const_field_list *)malloc(sizeof(ac_const_field_list));
  cfield->next = NULL;

  cfield->value = iconst_field;
  cfield->field.name = (char *)malloc(strlen(pfield->name)+1);
  strcpy(cfield->field.name, pfield->name);
  cfield->field.size = pfield->size;
  cfield->field.first_bit = pfield->first_bit;
  cfield->field.id = 0; /* not used */


  if (current_insn->const_fields == NULL) {
    current_insn->const_fields = cfield;
    current_insn->const_fields->next = NULL;
  }
  else {
    ac_const_field_list *head = current_insn->const_fields;
    while (head->next != NULL) head = head->next;
    head->next = cfield;
  }

  /* encode it, saving in 'c_image'
     note that there will be only 1 encoding for each format field, so it's ok to use the OR operation */
//  c_image |= ac_asm_encode_insn_field(iconst_field, pf, pfield);

  return 1;
}


static ac_const_field_list *clone_const_fields(ac_const_field_list *cfields)
{
  ac_const_field_list *lh = cfields;
  ac_const_field_list *fclone = NULL;

  while (lh != NULL) {
    ac_const_field_list *newfield = (ac_const_field_list *)malloc(sizeof(ac_const_field_list));
    newfield->next = NULL;

    newfield->value = lh->value;
    newfield->field = lh->field;

    if (fclone == NULL) {
      fclone = newfield;
    }
    else {
      ac_const_field_list *head = fclone;
      while (head->next != NULL) head = head->next;
      head->next = newfield;
    }

    lh = lh->next;
  }

  return fclone;
}


static void print_asm_insn(ac_asm_insn *insn)
{
  printf("\n--\nmnemonic = %s\n", insn->mnemonic);
  printf("\nliteral operand string = %s\n", insn->op_literal);

  printf("\noperands:\n\n");
  ac_operand_list *ops = insn->operands;
  while (ops != NULL) {
    printf("  string: %s\n", ops->str);
    printf("  type: ");

    switch (ops->type) {
      case op_userdef: printf("userdef\n"); break;
      case op_exp:     printf("exp\n"); break;
      case op_imm:     printf("imm\n"); break;
      case op_addr:    printf("addr\n"); break;
      default:      printf("error");
    }

    printf("\n  modifier:\n");
    printf("    name: %s \n", ops->modifier.name);
    printf("    addend: %d", ops->modifier.addend);


    printf("\n\n  fields:\n");
    ac_asm_insn_field *flist = ops->fields;
    while (flist != NULL) {
       printf("    name: %s (id = %d)\n", flist->name, flist->id);

      flist = flist->next;
    }
    printf("\n");

    ops = ops->next;
  }

  /* insn skipped */

  printf("\nconst fields:\n");
  ac_const_field_list *cf = insn->const_fields;
  while (cf != NULL) {
    printf("  value = %d | name = %s\n", cf->value, cf->field.name);
    /* field skipped */
    cf = cf->next;
  }

  printf("\npseudo list:\n");
  strlist *sl = insn->pseudolist;
  while (sl != NULL) {
    printf("  %s\n", sl->str);
    sl = sl->next;
  }

  printf("\nnumber of pseudos: %ld\n\n", insn->num_pseudo);

}


/*
  This should be called after the syntax string and argument field are
  processed.
  Strings 'formatted_asm_str' and 'formatted_arg_str' are merged
  and a new acpp_asm_insn is inserted in the insn list. 

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

  if (p == NULL) { /* pseudo instruction */
    if (pseudo_list == NULL) {
      sprintf(error_msg, "No pseudo instructions declared");
      return 0;
    }
    /* create a dummy field for every operand */
    ac_operand_list *opP = current_insn->operands;
    while (opP != NULL) {
      ac_asm_insn_field *newfield = (ac_asm_insn_field *)malloc(sizeof(ac_asm_insn_field));

      newfield->name = NULL;
      newfield->size = 0;
      newfield->first_bit = 0;
      newfield->id = 0;
//      newfield->reloc_id = 0;
      newfield->next = NULL;

      opP->fields = newfield;

      opP = opP->next;
    }
  }


  /**********
   Creates the mnemonic (creates the ac_asm_insn)
  ***********/

  
  /* creates a new ac_asm_insn  */

  ac_asm_insn *insn = (ac_asm_insn *) malloc(sizeof(ac_asm_insn));

  insn->op_literal = (char *)malloc(strlen(current_insn->op_literal)+1);
  strcpy(insn->op_literal, current_insn->op_literal);
  insn->op_literal_unformatted = (char *)malloc(strlen(current_insn->op_literal_unformatted)+1);
  strcpy(insn->op_literal_unformatted,current_insn->op_literal_unformatted);

  insn->operands = current_insn->operands;
  insn->insn = p;
  insn->const_fields = clone_const_fields(current_insn->const_fields);
  insn->pseudolist = pseudo_list;
  insn->num_pseudo = num_pseudo_insn;
  insn->next = NULL;

  insn->mnemonic = (char *) malloc(strlen(current_insn->mnemonic)+1);
  strcpy(insn->mnemonic, current_insn->mnemonic);
  
  //print_asm_insn(insn);


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
      while ((t != NULL) && ( (strcmp(insn->mnemonic, t->mnemonic) >= 0))) {
        t = t->next;
        last = last->next;
      }
      insn->next = t;
      last->next = insn;
    }
  }
  
  /* This code does the same as above, but keeping a second list in a
     different ordering. */
  
  ac_asm_insn *insn_clone = (ac_asm_insn *) malloc(sizeof(ac_asm_insn));

  insn_clone->op_literal = (char *)malloc(strlen(current_insn->op_literal)+1);
  strcpy(insn_clone->op_literal, current_insn->op_literal);
  insn_clone->op_literal_unformatted = (char *)malloc(strlen(current_insn->op_literal_unformatted)+1);
  strcpy(insn_clone->op_literal_unformatted,current_insn->op_literal_unformatted);

  insn_clone->operands = current_insn->operands;
  insn_clone->insn = p;
  insn_clone->const_fields = clone_const_fields(current_insn->const_fields);
  insn_clone->pseudolist = pseudo_list;
  insn_clone->num_pseudo = num_pseudo_insn;
  insn_clone->next = NULL;

  insn_clone->mnemonic = (char *) malloc(strlen(current_insn->mnemonic)+1);
  strcpy(insn_clone->mnemonic, current_insn->mnemonic);

  if (asm_insn_list_original_order == NULL) {
    asm_insn_list_original_order = insn_clone;
  } else {

    ac_asm_insn *t = asm_insn_list_original_order;
    ac_asm_insn *last = asm_insn_list_original_order;

    if ((insn_clone->insn == NULL && strcmp(insn_clone->mnemonic, t->mnemonic) < 0) ||
        (insn_clone->insn != NULL && strcmp(insn_clone->insn->name, t->insn->name) < 0)) { /* insert in the head */
      insn_clone->next = asm_insn_list_original_order;
      asm_insn_list_original_order = insn_clone;
    }
    else {
      t = t->next;
      /* keep the order in the source file */
      while ((t != NULL) && ( (insn_clone->insn == NULL && (strcmp(insn_clone->mnemonic, t->mnemonic) >= 0)) 
	|| (insn_clone->insn != NULL && (strcmp(insn_clone->insn->name, t->insn->name) >= 0)) )) {
        t = t->next;
        last = last->next;
      }
      insn_clone->next = t;
      last->next = insn_clone;
    }
  }
  
//  exit(-1);

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
//  int i;
//  for (i=0; i<num_args_expected; i++) {
//    formatted_arg_str[i*2] = '0';
//    formatted_arg_str[i*2+1] = ':';
//  }
//  formatted_arg_str[num_args_expected*2] = '\0';
}



/*
  This function validates de string 'pseudo' (by calling
acpp_asm_parse_asm_str with pseudo_field set) and then inserts the formatted
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

  if (pseudo_list == NULL) {
    pseudo_list = sl;
    pseudo_list->next = NULL;
  }
  else {
    while (pL->next != NULL) pL = pL->next;
    pL->next = sl;
  }

  num_pseudo_insn++;

  return 1;
}
