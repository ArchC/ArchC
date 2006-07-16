/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* ___arch_name___`-opc.c' -- ___arch_name___ opcode list.
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

#include <stdio.h>
#include "sysdep.h"
#include `"opcode/'___arch_name___`.h"'

acasm_opcode opcodes[] = {
___opcode_table___
};

const acasm_symbol udsymbols[] = {
___symbol_table___
};

const char *pseudo_instrs[] = {
___pseudo_table___
};


void print_opcode_structure(FILE *stream, unsigned int indent, acasm_opcode *insn)
{
  fprintf(stream, "%*s<\"%s\", \"%s\", 0x%08lX, %lu, %lu, %lu, 0x%08lX>\n",
                   indent, "",
                   insn->mnemonic,
                   insn->args,
                   insn->image,
                   insn->format_id,
                   insn->pseudo_idx,
                   insn->counter,
                   insn->dmask);
}





const int num_opcodes = ((sizeof opcodes) / (sizeof(opcodes[0])));
const int num_symbols = ((sizeof udsymbols) / (sizeof(udsymbols[0])));
const int num_pseudo_instrs = ((sizeof pseudo_instrs) / (sizeof(pseudo_instrs[0])));
