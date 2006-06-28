/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* ___arch_name___`.h.'  ___arch_name___ opcode list.
   Copyright 2005, 2006 --- The ArchC Team

This file is automatically retargeted by ArchC binutils generation tool. 
This file is part of GDB, GAS, and the GNU binutils.

GDB, GAS, and the GNU binutils are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
1, or (at your option) any later version.

GDB, GAS, and the GNU binutils are distributed in the hope that they
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * written by:
 *   Alexandro Baldassin            
 */

#ifndef _OPC_H_FILE_
#define _OPC_H_FILE_

/* This enum must be synced with acpp */
typedef enum {op_userdef, op_exp, op_imm, op_addr} operand_type;

/* This enum will be automatically generated in future versions */
typedef enum {mod_default, mod_low, mod_high, mod_aligned, mod_pcrel, mod_carry} operand_modifier;

typedef struct {
//  unsigned int id;   /* operand id */
  const char *name;  /* operand type name */
  operand_type type;
  operand_modifier mod_type;
  unsigned int mod_addend;
  unsigned int fields;
  unsigned int reloc_id;
} acasm_operand;

typedef struct {
  const char *mnemonic;
  const char *args;
  unsigned long image;
  unsigned long format_id;
  unsigned long pseudo_idx;
  unsigned long counter;
  unsigned long dmask;
} acasm_opcode;

typedef struct {
  const char *symbol;
  const char *cspec;
  unsigned long value;
} acasm_symbol;

extern void print_operand_info(FILE *stream, unsigned int indent, unsigned int opid);
extern void print_opcode_structure(FILE *stream, unsigned int indent, acasm_opcode *insn);

extern long long getbits(unsigned int bitsize, char *location, int endian); 
extern void putbits(unsigned int bitsize, char *location, long long value, int endian);

/* this will be generated */
extern void modifier_R(unsigned int input, unsigned int address, int addend, unsigned int *imm);
extern unsigned int get_num_fields(unsigned int encoded_field);
extern unsigned int get_field_id(unsigned int encoded_field, unsigned int pos);

extern unsigned int operand_buffer[32];

extern const acasm_operand operands[];
extern const unsigned int num_oper_id;
extern acasm_opcode opcodes[];
extern const int num_opcodes;
extern const acasm_symbol udsymbols[];
extern const int num_symbols;
extern const char *pseudo_instrs[];
extern const int num_pseudo_instrs;

#endif
