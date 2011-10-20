/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* Print ___arch_name___ instructions for GDB, the GNU debugger, or for objdump.
   Copyright 2005, 2006 --- The ArchC Team

This file is automatically retargeted by ArchC binutils generation tool. 
This file is part of GDB, GAS, and the GNU binutils.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * written by:
 *   Alexandre Keunecke I. de Mendonca            
 *   Felipe Guimaraes Carvalho                    
 *   Max Ruben de Oliveira Schultz                
 */


#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opintl.h"
#include `"opcode/'___arch_name___`.h"'
#include `"share-'___arch_name___`.h"'


/*----------------------------------------------------------------------------*/

#define `MAX_FORMAT_SIZE '___max_format_size___
#define `VARIABLE_FORMAT_SIZE '___variable_format_size___

/*----------------------------------------------------------------------------*/

// Disassembler generation definitions...

/*
  Linked list with all the operands of the isntruction.
  oper_id -> position of field in the instruction, later used to know about the size of field
  type -> type of field e.g. reg, addr, immm, exp
*/
typedef struct _ac_symbol {
  const char *type;
  int is_list;
  unsigned int oper_id;
  struct _ac_symbol *next;
} ac_symbol;


struct private
{
  /* Points to first byte not fetched.  */
  bfd_byte * max_fetched;
  bfd_byte   the_buffer[MAX_FORMAT_SIZE];
  bfd_vma    insn_start;
  jmp_buf    bailout;
};

/*----------------------------------------------------------------------------*/

/*
   bufInitial variable - set only by parse()
   Don't ever change these variables elsewhere!
*/
char bufInitial[100];
char bufFinal[100];

/*-----------------------------------------------------------------------------------*/

static int disassemble (bfd_vma memaddr, struct disassemble_info *info, unsigned long insn, int insn_size);

int `print_insn_'___arch_name___` (bfd_vma memaddr, struct disassemble_info * info);'

ac_symbol* parse(char *args);

void replace(char *str, const char *old, char *new);

void decode_modifier(mod_parms *mp, unsigned int oper_id);

/*------------------------------------------------------------------------------------*/


#define FETCH_DATA(info, addr) \
  ((addr) <= ((struct private *)(info->private_data))->max_fetched \
   ? 1 : fetch_data ((info), (addr)))

static int
fetch_data (struct disassemble_info *info, bfd_byte *addr){
  int status;
  struct private *priv = (struct private *) info->private_data;
  bfd_vma start = priv->insn_start + (priv->max_fetched - priv->the_buffer);

 

  status = (*info->read_memory_func) (start,
				      priv->max_fetched,
				      addr - priv->max_fetched,
				      info);
  /* verify (status != 5) because for architecture with variable format size are read 8, 16, 24 and 32 bits of memory and in the last instruction error can occur */
  if ((status != 0) && (status != 5)){
      (*info->memory_error_func) (status, start, info);
      longjmp (priv->bailout, 1);
  }
  else
    priv->max_fetched = addr;

  return 1;
}

/* 
 * Every bit set as one, marks the number encoded in the bitvector 
 * For example: 0x80000000 => 31 
 */ 
unsigned int get_bit_pos(unsigned int bitvector, unsigned int pos) {
  unsigned curr_bit = 0; pos++;
  while(1) {
    if (bitvector & (1 << curr_bit))
      if (!(--pos))
        return curr_bit;
    curr_bit++;
  }
}

unsigned long get_value_from_fields(unsigned int oper_id, 
                                    unsigned long insn,
                                    unsigned long *bit_size) {

  unsigned int i=0, j=0, mask=0, size=0, final_value=0;
  unsigned int num_fields = get_num_fields(operands[oper_id].fields);

  for (; i < num_fields; i++) {
    /* If we don't encode both start and end bits, we don't know 
       which field a start bit is refering to */
    unsigned field_start = get_bit_pos(operands[oper_id].fields_positions, i*2);
    unsigned field_end = get_bit_pos(operands[oper_id].fields_positions, (i*2)+1);
    unsigned field_size = field_end-field_start+1;

    /* Create a mask to extract the needed bits from the isns, and then
       concatenate disjoint fields and make them printable */
    for (j=0, mask=0; j < field_size; j++)
      mask |= (1 << (field_start + j));
    unsigned value = (insn & mask) >> field_start;
    
    /* This function considers that disjoint fields need to be
       concatenated in the order they appear. For different behavior
       one should implement custom handling via ac_modifiers. */
    final_value |= (value << size);
    size += field_size;
  }

  *bit_size = size;
  return final_value;
}

long sign_extend_to_long(long v, unsigned int bit_size) {
  long mask = 1ULL << (bit_size - 1);
  v = v & ((1ULL << bit_size) - 1);
  return (v ^ mask) - mask;
}

/*------------------------------------------------------------------------------------*/

/*
  Function that makes the disassembler of an instruction of the object file, making the decoding using the field dmask of the file xxxxx-opc.c

1- Using the insn apllies the mask (dmaks fiels of opcodes)
2- Search in instruction table the corresponding instruction of (insn & dmask), and prints the intruction mnemonic.
3- For each operand, is generated a mask in the position of the field in insn and size (e.g. bit 16 to the 24), after verified if is a register, or immediate, address, etc...
4- if is a register, is called the method replace to put the name of the register in the final string that it will be printed 
5- is an immediate one!,  then it verifies the modifiers, and it applies them, after calls the method replace to put the value in the final string(bufFinal). 
6- prints string(bufFinal) in the screen with the disassembled instruction.  
7- returns the octets processed, to be able to increment the address (PC) 

Params
  memaddr - address of the current instruction (PC value)
  info - structure of binutils used to print the instruction and operands
  insn - the raw instruction
*/
static int disassemble (bfd_vma memaddr, struct disassemble_info *info, unsigned long insn, int insn_size){
  acasm_opcode *op = (acasm_opcode *) opcodes;
  acasm_symbol *symbols = (acasm_symbol *) udsymbols;
  unsigned int FORMAT_SIZE=0;
  int nInstr=0;
  unsigned addr_value = 0;
  long imm_value = 0;

  //Find the mnemonic of instruction in opcodes table
  while (nInstr < num_opcodes){
    unsigned long e = insn & op->dmask;

    if ((e == op->image) && (op->pseudo_idx == 0) && ((unsigned)insn_size == get_insn_size(op->format_id))) {

      FORMAT_SIZE = get_insn_size(op->format_id);

      //Print instruction mnemonic
      (*info->fprintf_func) (info->stream,"%s",op->mnemonic);

      //Now look for operands
      char *args = malloc(sizeof(char)*50);
      strcpy(args, op->args);
      ac_symbol* acs = parse(args);

      char *ptInitial = bufInitial;
      char *ptFinal = bufFinal;      
      strcpy(ptFinal,ptInitial);

      while (acs){
        unsigned long size = 0;
        unsigned int numf = get_num_fields(operands[acs->oper_id].fields);
        unsigned int count, fid;
	symbols = (acasm_symbol *) udsymbols;
	int cont = 0;
	int in = 0;
	char symbolaux[100];
        fid = 0;

        /* Get the value that should be printed from the instruction */
        unsigned int bit_size = 0;
        unsigned long value = get_value_from_fields(acs->oper_id, insn, &bit_size);	

	if (acs->is_list){
	  node_list_op_results *p_list;
	  
	  //verify list decode modifier
	  mod_parms mp;
	  mp.input = value;
	  mp.address = memaddr;
	  mp.addend = operands[acs->oper_id].mod_addend;
	  mp.list_results = NULL;
	  decode_modifier(&mp, acs->oper_id);
	  if (mp.list_results != NULL) {
	    p_list = mp.list_results;	    
	    int leng = 0;
	    while (p_list) {	 	      
	      cont = 0;
	      in = 0;
	      symbols = (acasm_symbol *) udsymbols;
	      while (cont < num_symbols){
		if ((strcmp(acs->type, symbols->cspec) == 0) && (p_list->result == symbols->value)){//operand find
		  in = 1;
		  strcpy(&(symbolaux[leng]), symbols->symbol);		  
		  leng += strlen(&(symbolaux[leng]));
		  if (p_list->next != NULL)
		  {
		    symbolaux[leng] = ',';
		    symbolaux[leng+1] = '\0';
		    leng++;
		  }
		  break;
		}
		symbols++;
		cont++;
	      }
	      if (!in) {
		sprintf(symbolaux, "0x%x", p_list->result);
	      }	      	      
	      p_list = p_list->next;
	    }// fim while (p_list)
	    replace(bufFinal, acs->type, symbolaux);
	    free_list_results(&(mp.list_results));
	  }// if (mp.list_results != NULL)
	  else {
	    sprintf(symbolaux, "0x%lx", value);
	    replace(bufFinal, acs->type, symbolaux);
	  }// fim if
        // if (acs->is_list)
	} else {	  

	  if (strcmp(acs->type, "exp") && strcmp(acs->type, "imm") && strcmp(acs->type, "addr"))
	  {
	    /* Search the register table, looking value (taken from the dmask) is a register and finds the
	    name of the register and prints it as operating of instruction, if not found, will be an
	    immediate field or another thing it prints the value directly,*/
    
	    while (cont < num_symbols){
	      if ((strcmp(acs->type, symbols->cspec) == 0) && (value == symbols->value)){//operand find
		in = 1;
		strcpy(symbolaux, symbols->symbol);
	      }
	      symbols++;
	      cont++;
	    }
    
	    if (in){
	      replace(bufFinal, acs->type, symbolaux);
	    } else{
	      // Could not find operand symbol
	      sprintf(symbolaux, "0x%lx", value);
	      replace(bufFinal, acs->type, symbolaux);
	    }
	  } else {
            //operand type is: exp, imm or addr
  
	    //verify decode modifier
	    mod_parms mp;
	    mp.input = value;
	    mp.address = memaddr;
	    mp.addend = operands[acs->oper_id].mod_addend;
	    mp.list_results = NULL;

            // Put the instruction on the addend so the
            // modifier can play around in several ways.
            if (!mp.addend)
              mp.addend = insn;

	    decode_modifier(&mp, acs->oper_id);
	    if (mp.output != 0)
	      value = mp.output;	    
  
	    char *ptr3;
	    ptr3 = bufFinal;
	    char new[50];
	    //Convert long to char
	    sprintf( new, "0x%01lx", value );
	    replace(ptr3,acs->type,new);

            if (!(strcmp(acs->type, "imm")))
              imm_value = sign_extend_to_long(value, bit_size);  
	    
	    // if type is exp or addr, save its value. it can reference a symbol, and
	    // we want to print it as comment, eg. mov    r0, 0x10203    ; 10203 <.helloword>
	    if (! (strcmp(acs->type, "exp") && strcmp(acs->type, "addr")) )
	      addr_value = value;
	  }// end if (strcmp(acs->type, "exp") && strcmp(acs->type, "imm") && strcmp(acs->type, "addr"))
	}//end if (acs->is_list)
        acs = acs->next;
      }
      break;
    }
    op++;
    nInstr++;
  }
 
  //.data Section
  if (FORMAT_SIZE == 0) {
    FORMAT_SIZE = insn_size;
    //Convert long to char
    sprintf( bufFinal, "0x%01lx", insn );
  }

  (*info->fprintf_func) (info->stream,"%s",bufFinal);
  if (addr_value) {
    (*info->fprintf_func) (info->stream,"\t; ", addr_value);
    info->print_address_func (addr_value, info);
  } else if (imm_value != 0) {
    (*info->fprintf_func) (info->stream,"\t; %ld", imm_value);
  }
  return  FORMAT_SIZE / 8;
}
/*------------------------------------------------------------------------------------*/

/*
   Function that is called by objdump core (entry point for the disassemble) for each instruction of the file, reads from the binary file the raw insn and calls the function disassemble() that disassemble the instruction.
*/
int `print_insn_'___arch_name___` (bfd_vma memaddr, struct disassemble_info * info){'
  struct private priv;
  bfd_byte *buffer = priv.the_buffer;
  unsigned long insn = 0;
  unsigned long insnfound;
  int sizeinsn;

  info->private_data = & priv;
  priv.max_fetched = priv.the_buffer;
  priv.insn_start = memaddr;

  if (setjmp (priv.bailout) != 0) /* Error return.  */
    return -1;

  acasm_opcode *op = (acasm_opcode *) opcodes;

  /* Verify if architecture has variable format size (CISC) ou no (RISC) */
  if (VARIABLE_FORMAT_SIZE == 0) {
    /*RISC - read fix length bits of the memory */
    FETCH_DATA (info, buffer + (MAX_FORMAT_SIZE / 8));
    insn = getbits(MAX_FORMAT_SIZE, (char *)buffer, ___endian_val___);
    insnfound = insn;
    sizeinsn = MAX_FORMAT_SIZE;
  }
  else {
    /* CISC - read variable length bits of the memory
       1- read 8 bits and find instruction in opcodes table
       2- if has format size > 8, read 16 bits and find instruction in opcodes table
       3- if has format size > 16, read 24 bits and find instruction in opcodes table
       4- if has format size > 24, read 32 bits and find instruction in opcodes table
       Decode last insn find.
    */
    unsigned int localsize = 8;
    unsigned int i;
    while (localsize <= MAX_FORMAT_SIZE) {
      FETCH_DATA (info, buffer + (localsize / 8));
      for (i=3; i>=(localsize/8); i--)
        buffer[i] = 0;
      insn = getbits(localsize, (char *)buffer, ___endian_val___);

      //Find the mnemonic of instruction in opcodes table
      int j = 0;     
      while ((j < num_opcodes) && (op->dmask != 0)) {
        unsigned long e = insn & op->dmask;

        if ((e == op->image) && (op->pseudo_idx == 0) && (localsize == get_insn_size(op->format_id))){
          insnfound = insn;
          sizeinsn = localsize;
          break;
        }
        op++;
        j++;
      }
      localsize = localsize + 8;
      op = op - j;
    }
  }

  info->bytes_per_chunk = (sizeinsn / 8);

  return disassemble(memaddr, info, insnfound, sizeinsn);
}
/*------------------------------------------------------------------------------------*/

/*
Example:
input args = %0:,%1:,%2:
output bufInitial = reg,reg,imm
*/
ac_symbol* parse(char *args){
  char *ptr_bufInitial = bufInitial;
  ac_symbol *lista = NULL;
  ac_symbol *temp;
  ac_symbol *t;
  t = NULL;
  temp = NULL;
  int is_mnemonic_suffix = 1;
  int i = 0;

  //loop in the chars of args
  while (*args != '\0'){
    char buf[50];
    char *ptr_buf = buf;

    //symbol '%' indicate a initial field
    if (*args == '%'){
      args++;
      //symbol ':' indicate a final field
      while ((*args != ':') && (*args != '+')){
        *ptr_buf = *args;
        *ptr_bufInitial = '\0';
        ptr_buf++;
        args++;
      }
      *ptr_buf = '\0';
      args++; 

      //operand_id is the index of fiels in the struct operands
      unsigned int operand_id = atoi(buf);
      unsigned int count;

      //replace index of field the bufInitial to type of field e.g. reg, addr, immm, exp
      strcat(ptr_bufInitial, operands[operand_id].name);
      for (count=0; count < strlen(operands[operand_id].name); count++)
        ptr_bufInitial++;

      //verify aditional fiels
      if (*args == '+') {
        strcat(ptr_bufInitial, "+");
        ptr_bufInitial++;
        args++;
      }

      if (i > 0){ //more elements of list
        temp = (ac_symbol *) malloc(sizeof(ac_symbol));
        temp->oper_id = operand_id;
	temp->is_list = operands[operand_id].is_list;
        temp->type = operands[operand_id].name;
        temp->next = NULL;
        lista->next = temp;
        lista = lista->next;
      }
      else{ //first element of list
        lista = (ac_symbol *) malloc(sizeof(ac_symbol));
        t = lista;
        lista->oper_id = operand_id;
	lista->is_list = operands[operand_id].is_list;
        lista->type = operands[operand_id].name;
        lista->next = NULL;
      }
      i++;
    }
    else{  //if (*args=='%')          
      if (*args == '\\'){ //ignore char "\\"
        args++;
        args++;
        //ignore char "lo", "hi"...
        while ((*args>64 && *args<91) || (*args>96 && *args<123))
          args++;
      }
      else{
	if (*args == ' ' && is_mnemonic_suffix)
	{
	  is_mnemonic_suffix = 0;
	  *ptr_bufInitial = '\t';
	  args++;
	  ptr_bufInitial++;
	}
	else
	{
          *ptr_bufInitial = *args;
          args++;
          ptr_bufInitial++;
	}
      }
    }
  }//end of while (*args!='\0')

  *ptr_bufInitial = '\0';
  lista = t;
  return lista;
}
/*----------------------------------------------------------------------------*/

/*
   Function that from parameter 'ptr_ret' search for the first occurrence of 'old' and substitutes for 'new' 
   e.g.: ptr_ret = addi reg,reg,exp
       old = reg
       new = $30
   result:
       ptr_ret = addi $30,reg,exp
*/
void replace(char *ptr_ret, const char *old, char *new){
  char a[50];
  char *str = a;

  strcpy(str, ptr_ret);

  while (*str != '\0'){
    if (strncmp(old, str, strlen(old)) == 0){
      while (*old == *str){
        old++;
        if (*str != '\0')
        str++;
      }
      while (*new != '\0'){
        *ptr_ret = *new;
        ptr_ret++;
        new++;
      }
      while (*str != '\0'){
        *ptr_ret = *str;
        str++;
        ptr_ret++;
      }
    }
    else{
      *ptr_ret = *str;
      str++;
      ptr_ret++;
    }
  }

  *ptr_ret = '\0';
}
/*------------------------------------------------------------------------------------*/

/* Verify if has decode code for the modifier */
void decode_modifier(mod_parms *mp, unsigned int oper_id){
  mp->output = 0;
  if (operands[oper_id].mod_type != mod_default)
     (*modfndec[operands[oper_id].mod_type])(mp);
}
/*------------------------------------------------------------------------------------*/
