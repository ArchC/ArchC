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

extern void print_opcode_structure(FILE *stream, unsigned int indent, acasm_opcode *insn);

extern acasm_opcode opcodes[];
extern const int num_opcodes;
extern const acasm_symbol udsymbols[];
extern const int num_symbols;
extern const char *pseudo_instrs[];
extern const int num_pseudo_instrs;

#endif
