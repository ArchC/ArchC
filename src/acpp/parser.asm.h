/* ex: set tabstop=2 expandtab: */
/* 
    ArchC parser (asm module) - Copyright (C) 2002-2005  The ArchC Team

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
/* parser.asm.h: ArchC parser (asm module)  .           */
/* Author: Alexandro Baldassin                          */
/* Date: 01-06-2005                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

/*! \file parser.asm.h
 *  \brief ArchC parser for asm-related constructs
 *  This file contains the structures and interface 
 *  functions with the ArchC grammar (bison file)
 */

/** @defgroup acppasm asm-related parser 
 * @ingroup acpp
 *
 * @{
 */

#ifndef _PARSER_ASM_H_
#define _PARSER_ASM_H_

#include "ac_decoder.h"

/*
  A general structure to hold string list. Not sure if here is the best place
  to put it.
*/
typedef struct _strlist {
  char *str;
  struct _strlist *next;
} strlist;



/**********************************************

 ACASM parser structures

**********************************************/

/*
  Structure used to hold a symbol element in an 'ac_asm_map' declaration.
*/
typedef struct _ac_asm_symbol {
  char *symbol;                     /* the symbol name */
  int value;                        /* symbol's value */
  struct _ac_asm_symbol *next;      /* pointer to next symbol element */
} ac_asm_symbol;


/*
  Structure used to hold an 'ac_asm_map' declaration. It's made up of a marker
(a mapping name), and a symbol list representing the mapping between strings
and values.
*/
typedef struct _ac_asm_map_list {
  char *marker;                    /* mapping name - must be unique */
  int used_where;                  /* 0 - not used, 1 - operand, 2 - mnemonic, 3- both of them */
  ac_asm_symbol *symbol_list;      /* symbol mapping list */
  struct _ac_asm_map_list *next;   /* pointer to next ac_asm_map element */
} ac_asm_map_list;


typedef enum {op_userdef, op_exp, op_imm, op_addr} operand_type;
typedef enum {mod_low, mod_high, mod_aligned, mod_pcrel, mod_pcrelext} operand_modifier;

typedef struct _ac_asm_insn_field {
  char *name;
  unsigned int size;
  unsigned int first_bit;
  unsigned int id;
  unsigned int reloc_id;
  struct _ac_asm_insn_field *next;
} ac_asm_insn_field;

typedef struct _ac_modifier_list {
  operand_modifier type;
  unsigned int addend;
  unsigned int sign;       /* 0 - unsigned, 1 - signed */ 
  unsigned int carry;      /* 0 - no carry, 1 - carry */
  struct _ac_modifier_list *next;  
} ac_modifier_list;

typedef struct _ac_operand_list {
  char *str;                      /* operand string */
  operand_type type;              /* operand type */
  ac_modifier_list *modifiers;    /* modifiers assigned to this operand */
  ac_asm_insn_field *fields;      /* a chain of fields this operand is assigned to */
  struct _ac_operand_list *next;
} ac_operand_list;

typedef struct _ac_const_field_list {
  unsigned int value;
  ac_asm_insn_field field;
  struct _ac_const_field_list *next;
} ac_const_field_list;

/*
  It's the main asm structure. It represents an instruction (insn) as seen by
  the asm module.  You can think of it as an expansion of the ac_dec_instr
  structure, used by the decoder.
*/
typedef struct _ac_asm_insn {
  char *mnemonic;             /* mnemonic part of asm syntax */
  char *op_literal;             
  ac_operand_list *operands;
  ac_dec_instr *insn;         /* pointer to original ac_dec_instr - NULL if it's a pseudo insn */
  ac_const_field_list *const_fields;
  strlist *pseudolist;        /* if a pseudo insn, it holds a list of the insns that make up the pseudo */
  long num_pseudo;            /* number of pseudo insns - only for speed reasons */
  unsigned reloc_id;         /* Relocation id */
  struct _ac_asm_insn *next;  /* pointer to next element */
} ac_asm_insn;



/**********************************************

 ACASM parser exported functions
 For further hints, look at the .c file

**********************************************/


/*
 Parser interface functions
*/

/* ac_asm_map relative functions */
extern int acpp_asm_create_mapping_block(char *marker, char *error_msg);
extern int acpp_asm_add_mapping_symbol(char *symbol, char *error_msg);
extern int acpp_asm_add_mapping_symbol_range(char *sb, char *sa, int r1, int r2, char *error_msg);
extern int acpp_asm_add_symbol_value(int val1, int val2, char *error_msg);


/* set_asm relative functions */
extern void acpp_asm_new_insn(); /* also used by pseudo_op */ 
extern int acpp_asm_parse_asm_string(char *asm_str, int is_pseudo, char *error_msg); /* also used by pseudo_op */
extern int acpp_asm_parse_asm_argument(ac_dec_format *pf, char *field_str, int is_concatenated, char *error_msg);
extern int acpp_asm_parse_const_asm_argument(ac_dec_format *pf, char *field_str, int iconst_field, char *sconst_field, char *error_msg);
extern int acpp_asm_end_insn(ac_dec_instr *p, char *error_msg); /* also used by pseudo_op */


/* pseudo_op relative functions */
extern void acpp_asm_new_pseudo();
extern int acpp_asm_add_pseudo_member(char *pseudo, char *error_msg);




/*
 ASM-related interface functions
*/

/* gets the pointer to the asm insn list generated by the parser */
extern ac_asm_insn* ac_asm_get_asm_insn_list();

/* gets the pointer to the asm map list generated by the parser */
extern ac_asm_map_list* ac_asm_get_mapping_list();

/** @} */

#endif /* _PARSER_ACASM_H */
