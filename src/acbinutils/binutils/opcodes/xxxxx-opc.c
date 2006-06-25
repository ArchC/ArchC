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

const acasm_operand operands[] = {
___operand_table___
};


void print_opcode_structure(FILE *stream, acasm_opcode *insn)
{
  fprintf(stream, "Mnemonic: \"%s\"\n", insn->mnemonic);
  fprintf(stream, "Args    : \"%s\"\n", insn->args);
  fprintf(stream, "Image   : 0x%lX\n", insn->image);
  fprintf(stream, "Format  : %lu\n", insn->format_id);
  fprintf(stream, "Pseudo  : %lu\n", insn->pseudo_idx);
  fprintf(stream, "Counter : %lu\n", insn->counter);
  fprintf(stream, "Dmask   : 0x%lX\n", insn->dmask);
}

void print_operand_info(FILE *stream, unsigned int opid)
{
  fprintf(stream, "Name   :\"%s\"\n", operands[opid].name);
  fprintf(stream, "Type   : ");
  switch (operands[opid].type) {
    case op_exp: fprintf(stream, "exp\n");
      break; 
    case op_imm: fprintf(stream, "imm\n");
      break;
    case op_addr: fprintf(stream, "addr\n");
      break;
    case op_userdef: fprintf(stream, "userdef\n");
      break;
  }
  fprintf(stream, "ModType : ");
  switch (operands[opid].mod_type) {
    case mod_default: fprintf(stream, "mod_default\n");
      break;
    case mod_low: fprintf(stream, "mod_low\n");
      break;
    case mod_high: fprintf(stream, "mod_high\n");
      break;
    case mod_aligned: fprintf(stream, "mod_aligned\n");
      break;
    case mod_pcrel: fprintf(stream, "mod_pcrel\n");
      break;
    case mod_carry: fprintf(stream, "mod_carry\n");
      break;
  }
  fprintf(stream, "Fields: 0x%X\n", operands[opid].fields);
  unsigned int numf = get_num_fields(operands[opid].fields);
  fprintf(stream, "  Num Fields: %d\n", numf);
  fprintf(stream, "  Fields id: ");
  unsigned int count;
  for (count=0; count < numf; count++) {
    fprintf(stream, "%d ", get_field_id(operands[opid].fields, count));
  }
  fprintf(stream, "\n");

  fprintf(stream, "reloc id: %d\n", operands[opid].reloc_id);
}



/*
 * 1 = big, 0 = little
 *
 * limitations:
 * . endianess affects 8-bit bytes
 * . maximum word size: 64-bit
 */
long long getbits(unsigned int bitsize, char *location, int endian)
{
  long long data = 0;
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
	
  if (bits_remain) number_bytes++;

  int index, inc;
  if (endian == 1) { /* big */
    index = 0;
    inc = 1;		
  }
  else { /* little */
    index = number_bytes - 1;
    inc = -1;
  }
	
  while (number_bytes) {
    data = (data << 8) | (unsigned char)location[index];
		
    index += inc;
    number_bytes--;
  }	

/*
 * if bitsize is not multiple of 8 then clear/pack the remaining bits
 */
  long long mask = 0;
  for (; bitsize; bitsize--) 
    mask = (mask << 1) | 1;
	
  if (bits_remain) {
    if (endian == 1) {
      long long temp = data >> (8 - bits_remain);
      temp &= (mask << bits_remain);
			
      data = temp | (data & ~(mask << bits_remain));
    }
  }
	
  return data & mask;
}


void putbits(unsigned int bitsize, char *location, long long value, int endian)
{
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
  unsigned char bytes[8];
  unsigned int i;
  int index;
  unsigned char mask = 0;

  /*
	* Fill the bytes array so as not to depend on the host endian
	*
	* bytes[0] -> least significant byte
	*/
  for (i=0; i<8; i++) 
    bytes[i] = (value & (0xFF << (i*8))) >> 8*i;

  if (bits_remain) {
    for (i=0; i<bits_remain; i++)
	   mask = (mask << 1) | 1;	
  }
	 
  if (endian == 0) { /* little */
    index = 0;

    while ((unsigned int)index != number_bytes) {    
      location[index] = bytes[index];
	  
      index++;
    }

	 if (bits_remain) 
     location[number_bytes] = (location[number_bytes] & ~mask) | (bytes[number_bytes] & mask);
	 	 
  }
  else { /* big */
    index = number_bytes - 1;
	
    i = 0;	 
    while (index >= 0) {    
      if (bits_remain) 
        location[i] = ((bytes[index+1] & mask) << (8-bits_remain)) | (bytes[index] >> bits_remain);
      else		 
        location[i] = bytes[index];

      i++;
      index--;
    }

    if (bits_remain) 
      location[number_bytes] = (bytes[0] & mask) | (location[number_bytes] & ~mask);
  }	
}

unsigned int get_num_fields(unsigned int encoded_field)
{
  /* count the number of 1's in the bit field */
  unsigned numf = 0;
  unsigned int shift = 1;

  for (; shift; shift <<= 1) {
    if (encoded_field & shift)
      numf++;
  }

  return numf;
}

unsigned int get_field_id(unsigned int encoded_field, unsigned int pos)
{
  unsigned int id = 999;
  unsigned int counter = 0;
  unsigned int shift = 1;

  for (; shift; shift <<= 1) {
    if (encoded_field & shift) {
      if (pos == 0) return counter;
      pos--;
    }
    counter++;
  }

  return id;
}



/*
 * Modifiers
 * NOTE: This will be generated
 */
void modifier_R (unsigned int input, unsigned int address, int addend, unsigned int *imm)
{

  /* user written code */
  *imm = input - address + addend;


}

/* will be automatically generated */
unsigned int operand_buffer[32];



const unsigned int num_oper_id = ((sizeof operands) / (sizeof(operands[0])));
const int num_opcodes = ((sizeof opcodes) / (sizeof(opcodes[0])));
const int num_symbols = ((sizeof udsymbols) / (sizeof(udsymbols[0])));
const int num_pseudo_instrs = ((sizeof pseudo_instrs) / (sizeof(pseudo_instrs[0])));
