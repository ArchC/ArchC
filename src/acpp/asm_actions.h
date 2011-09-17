/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      asm_actions.h
 * @author    Alexandro Baldassin
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
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/** @defgroup asmact_group Assembly semantic actions
 * @ingroup bison_group
 *
 * This module provides the functions called by the
 * language rules to deal with assembly-related constructs.
 *
 *
 * General overview of the constructs and the way things work.
 *
 * ********************
 *    ac_asm_map
 * ********************
 *
 * A list of the mappings declared is stored in the 'mapping_list' variable.
 * It can be retrieved through the 'ac_asm_get_mapping_list()' function.
 * It's up to the caller to get the info they need in the list, just be sure
 * you don't change it ;)
 *
 * Basic Parser Info:
 *
 * Upon finding the 'ac_asm_map' keyword, the parser should call
 * 'acpp_asm_create_mapping_block()' with the ID as an argument. This will
 * check for redefinition of the ID and add it to the internal list. After
 * this, each <symbol> found in the right side of the attribution must be
 * added calling either 'acpp_asm_add_mapping_symbol()' or
 * 'acpp_asm_add_mapping_symbol_range()' depending if the symbol include a
 * range [] or not. The value (or the range of values) can then be assigned
 * through the 'acpp_asm_add_symbol_value()' function. The dependency among
 *  the calls to the functions is:
 *
 * Step 1 - call acpp_asm_create_mapping_block() to create a mapping block
 *
 * Step 2 - call acpp_asm_add_mapping_symbol() or
 *          acpp_asm_add_mapping_symbol_range() to stack symbol definitions
 *          for that block
 *
 * Step 3 - call acpp_asm_add_symbol_value() to assign a value (or a range of
 *          values) to the symbol(s) stacked
 *
 * Step 4 - either add more symbols going to step 2 or create another mapping
 *          block going to step 1
 *
 *
 * ********************
 *    set_asm
 * ********************
 *
 * set_asm is tied up to an 'ac_instr' type. It's the type used by the ArchC
 * language so that instructions can be created.  Every insn declared by
 * 'ac_instr' can use a property called 'set_asm' to declared its assembly
 * syntax and encoding scheme.  'set_asm' resembles the standard C function
 * 'scanf': there's a string with literal and formatting characters (those
 * starting with the '%' char) and a list of arguments for each of the
 * formatting characters. The syntax string is not parsed by the bison
 * itself; it uses some of the functions implemented here.
 *
 *
 * A list of the insn syntaxes declared are stored in the 'asm_insn_list'
 * variable. It can be retrieved through the 'ac_asm_get_asm_insn_list()'
 * function. It's up to the caller to get the info they need in the list,
 * just be sure you don't change it ;)
 *
 * Basic Parser Info:
 *
 * When a 'set_asm' directive is found in the ArchC source file and its syntax
 * should be checked, you must call the init function 'acpp_asm_new_insn()'.
 * It initializes some internal states. After that, syntax strings are parsed
 * through a calling to the function 'acpp_asm_parse_asm_string()'. Arguments
 * and constant arguments are processed by calling 'acpp_asm_parse_asm_argument()'
 * or 'acpp_asm_parse_const_asm_argument()'. To insert the insn to the list,
 * call 'acpp_asm_end_insn()'. It's the last step when parsing the whole
 * 'set_asm' stuff. Those functions work by creating internal representation
 * of the strings being parsed. end_insn is responsable for creating the final
 * asm string and insert it into the asm_insn_list.
 *
 * The dependency among the calls to the functions is:
 *
 * Step 1 - call acpp_asm_new_insn() before any other function, to initialize
 *          internal states
 *
 * Step 2 - call acpp_asm_parse_asm_insn() to parse the syntax string
 *
 * Step 3 - either call acpp_asm_parse_asm_argument() or
 *          acpp_asm_parse_const_asm_argument() to process the each argument
 *
 * Step 4 - call acpp_asm_end_insn() to create a new ac_asm_insn and insert it
 *          in the list of insns
 *
 *
 * ********************
 *    pseudo_instr
 * ********************
 *
 * A pseudo insn is also stored in the 'asm_insn_list'. However, some fields
 * of the structure ac_asm_insn has some fixed values.  'mnemonic' and
 * 'operand' fields store the mnemonic and operands strings of
 * <pseudo_declaration>. 'insn' field is always NULL since there is no
 * ac_dec_instr attached to a pseudo. 'const_image' is always 0 since
 * pseudo-ops don't have arguments. 'pseudo_list' is a string list with all
 * <pseudo_member> as declared in the ArchC source file. 'num_pseudo' is the
 * number of pseudo members in the 'pseudo_list' field.
 *
 * Basic Parser Info:
 *
 * 'acpp_asm_parse_asm_insn()' is also used to parse <pseudo_declaration>
 * setting the flag 'is_pseudo' to 0 (like a native insn).  Only after that
 * one should call 'acpp_asm_new_pseudo()'. To insert each <pseudo_member>,
 * call 'acpp_asm_add_pseudo_member()'. It will insert them in the pseudo_list.
 * To finish, call 'acpp_asm_end_insn()' to insert it in the asm_insn_list.
 * The dependency among the calls to the functions is:
 *
 * Step 1 - call acpp_asm_parse_asm_insn() to parse the base pseudo-op string
 *           (<pseudo_declaration>)
   *
 * Step 2 - call acpp_asm_new_pseudo() to initialize internal states in
 *          pseudo-op processing
 *
 * Step 3 - call acpp_asm_add_pseudo_member() for each insn of the pseudo
 *           declaration
 *
 * Step 4 - call acpp_asm_end_insn() to insert all in the asm_insn_list
 *
 *
 *
 * @{
 */

#ifndef _ASM_ACTIONS_H_
#define _ASM_ACTIONS_H_

#include "ac_decoder.h"

/*!
 * A general structure to hold string list. Not sure if here is the best place
 * to put it.
 */
typedef struct _strlist {
  char *str;
  struct _strlist *next;
} strlist;



/*
 * Binary utilities parser structures
 */

/*!
 * Structure used to hold an 'ac_asm_map' declaration. It's made up of a marker
 * (a mapping name). It just stores information about the maps declared and no more
 * has pointers to symbols declared in it, since now this is stored in a global
 * alphabetically sorted linked list.
 */
typedef struct _ac_asm_map_list {
  char *marker;                    /*!< mapping name - must be unique */  
  struct _ac_asm_map_list *next;   /*!< pointer to next ac_asm_map element */
} ac_asm_map_list;


/*!
 * Structure used to hold a symbol element in an 'ac_asm_map' declaration.
 */
typedef struct _ac_asm_symbol {
  char *symbol;                     /*!< the symbol name */
  int value;                        /*!< symbol's value */
  char *map_marker;                 /*!< map name owning this symbol */
  struct _ac_asm_symbol *next;      /*!< pointer to next symbol element */
} ac_asm_symbol;




typedef enum {op_userdef, op_exp, op_imm, op_addr} operand_type;

typedef struct _ac_asm_insn_field {
  char *name;
  unsigned int size;
  unsigned int first_bit;
  unsigned int id;
  struct _ac_asm_insn_field *next;
} ac_asm_insn_field;

typedef struct _ac_modifier {
  char *name;
  int type;    /* not filled by the parser --- default = -1 */
  int addend;
} ac_modifier;

typedef struct _ac_operand_list {
  char *str;                      /*!< operand string */
  operand_type type;              /*!< operand type */
  ac_modifier modifier;           /*!< modifier assigned to this operand */
  ac_asm_insn_field *fields;      /*!< a chain of fields this operand is assigned to */
  int is_list;
  int oper_id;              /* not filled by parser, default = -1 */
  unsigned int reloc_id;    /* not filled by parser, default = 0 */
  struct _ac_operand_list *next;
} ac_operand_list;

typedef struct _ac_const_field_list {
  unsigned int value;
  ac_asm_insn_field field;
  struct _ac_const_field_list *next;
} ac_const_field_list;

/*!
 * It's the main asm structure. It represents an instruction (insn) as seen by
 * the asm module.  You can think of it as an expansion of the ac_dec_instr
 * structure, used by the decoder.
 */
typedef struct _ac_asm_insn {
  char *mnemonic;             /*!< mnemonic part of asm syntax */
  char *op_literal;
  char *op_literal_unformatted; /*!< Used by compiler generator to emit this insn */
  ac_operand_list *operands;
  ac_dec_instr *insn;         /*!< pointer to original ac_dec_instr - NULL if it's a pseudo insn */
  ac_const_field_list *const_fields;
  strlist *pseudolist;        /*!< if a pseudo insn, it holds a list of the insns that make up the pseudo */
  long num_pseudo;            /*!< number of pseudo insns - only for speed reasons */
//  unsigned reloc_id;          /*!< Relocation id */
  struct _ac_asm_insn *next;  /*!< pointer to next element */
} ac_asm_insn;



/*!
 * Binary utilities parser exported functions
 * For further hints, look at the .c file
 */

/*!
 * Parser interface functions
 */

/* special acasm declaration functions */
extern int acpp_set_assembler_comment_chars(char *input, char *error_msg);
extern int acpp_set_assembler_line_comment_chars(char *input, char *error_msg);
    
/* ac_asm_map relative functions */
extern int acpp_asm_create_mapping_block(char *marker, char *error_msg);
extern int acpp_asm_add_mapping_symbol(char *symbol, char *error_msg);
extern int acpp_asm_add_mapping_symbol_range(char *sb, char *sa, int r1, int r2, char *error_msg);
extern int acpp_asm_add_symbol_value(int val1, int val2, char *error_msg);


/* set_asm relative functions */
extern void acpp_asm_new_insn(); /* also used by pseudo_op */
extern int acpp_asm_parse_asm_string(char *asm_str, int is_pseudo, char *error_msg); /* also used by pseudo_op */
extern int acpp_asm_parse_asm_argument(ac_dec_format *pf, char *field_str, int is_concatenated, char *error_msg);
extern int acpp_asm_parse_const_asm_argument(ac_dec_format *pf, char *field_str, char *map, int iconst_field, char *sconst_field, char *error_msg);
extern int acpp_asm_end_insn(ac_dec_instr *p, char *error_msg); /* also used by pseudo_op */


/* pseudo_op relative functions */
extern void acpp_asm_new_pseudo();
extern int acpp_asm_add_pseudo_member(char *pseudo, char *error_msg);



/*!
 * Tool-related interface functions
 */

/*! gets the pointer to the asm insn list generated by the parser */
extern ac_asm_insn* ac_asm_get_asm_insn_list();

/*! gets the pointer to the asm insn list generated by the parser.
    This version is used when a backend needs the correct ordering
    of set_asm constructs as it appears in the ArchC source file. */
extern ac_asm_insn* ac_asm_get_asm_insn_list_original_order();

/*! gets the pointer to the asm map list generated by the parser */
extern ac_asm_symbol* ac_asm_get_mapping_list();

/*! gets string of chars used as commentary delimiters in assembly
    language. */
extern char * ac_asm_get_comment_chars();
/*! gets string of chars used as line commentaries delimiters in assembly
    language. */
extern char * ac_asm_get_line_comment_chars();


/*@}*/

#endif /* _ASM_ACTIONS_H */
