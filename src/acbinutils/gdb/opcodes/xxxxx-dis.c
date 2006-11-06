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
  char *type;
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
char bufInitial[50];
char bufFinal[50];

/*-----------------------------------------------------------------------------------*/

static int disassemble (bfd_vma memaddr, struct disassemble_info *info, unsigned long insn, int insn_size);

int `print_insn_'___arch_name___` (bfd_vma memaddr, struct disassemble_info * info);'

ac_symbol* parse(char *args);

void replace(char *str, char *old, char *new);

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

  //Find the mnemonic of instruction in opcodes table
  while (nInstr < num_opcodes){
    unsigned long e = insn & op->dmask;

    if ((e == op->image) && (op->pseudo_idx == 0) && ((unsigned)insn_size == get_insn_size(op->format_id))) {

      FORMAT_SIZE = get_insn_size(op->format_id);

      //Print instruction mnemonic
      (*info->fprintf_func) (info->stream,"%s\t",op->mnemonic);

      //Now Look for Registers and address
      char *args = malloc(sizeof(char)*50);
      strcpy(args, op->args);
      ac_symbol* acs = parse(args);

      char *ptInitial = malloc(sizeof(char)*50);
      char *ptFinal = malloc(sizeof(char)*50);
      ptInitial = bufInitial;
      ptFinal = bufFinal;
      strcpy(ptFinal,ptInitial);

      while (acs){
        unsigned long size = 0;
        unsigned int numf = get_num_fields(operands[acs->oper_id].fields);
        unsigned int count, fid;
        fid = 0;
        for (count=0; count < numf; count++){
          fid = get_field_id(operands[acs->oper_id].fields, count);
          size = size + get_field_size(op->format_id, fid);
        }

        unsigned int i = 0;
        unsigned long begining = 0;
        for (i=0; i<=fid; i++)
          begining += get_field_size(op->format_id, i);

        unsigned long dmask = 0x00;
        for (i=0; i<size; i++)
          dmask = (dmask << 1) + 1;

        //operand dmask
        `if ('___endian_val___`) ' //big
          dmask = dmask << (FORMAT_SIZE - begining);
        else //little
          dmask = dmask << (begining - size);

        unsigned long value = dmask & insn;

        //dislocates masks to see it which reg ex: [0,31]
        `if ('___endian_val___`) ' //big
          value = value >> (FORMAT_SIZE - begining);
        else //little
          value = value >> (begining - size);

        int cont = 0;
        int in = 0;
        char symbolaux[50];

        /* Search the register table, looking value (taken from the dmask) is a register and finds the name of the register and prints it as operating of instruction, if not found, will be an immediate field or another thing it prints the value directly,*/

        while (cont < num_symbols){
          if ((strcmp(acs->type, symbols->cspec) == 0) && (value == symbols->value)){//operand find
            in = 1;
            strcpy(symbolaux, symbols->symbol);
          }
          symbols++;
          cont++;
        }

        if (in){
          char *ptr2;
          ptr2 = bufFinal;
          replace(ptr2, acs->type, symbolaux);
        }
        else{//not operand find, it can be: exp, immm, addr

          signed long value_aux = value;
          //verify if is a negative value - two complement
          if (value & (1<<(size-1))){
            value_aux = (((0xFFFFFFFF >> (32 - size)) & ~value) + 1);
            value_aux = (value_aux * -1);
          }

          //verify decode modifier
          mod_parms mp;
          mp.input = value_aux;
          mp.address = memaddr;
          mp.addend = operands[acs->oper_id].mod_addend;
          decode_modifier(&mp, acs->oper_id);
          if (mp.output != 0)
            value = mp.output;

          char *ptr3;
          ptr3 = bufFinal;
          char new[50];
          //Convert long to char
          sprintf( new, "0x%01lx", value );
          replace(ptr3,acs->type,new);
        }
        symbols = symbols - cont;//Return table symbol pointer
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

  (*info->fprintf_func) (info->stream,"%s ",bufFinal);
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
    insn = getbits(MAX_FORMAT_SIZE, buffer, ___endian_val___);
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
      insn = getbits(localsize, buffer, ___endian_val___);

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
        temp->type = operands[operand_id].name;
        temp->next = NULL;
        lista->next = temp;
        lista = lista->next;
      }
      else{ //first element of list
        lista = (ac_symbol *) malloc(sizeof(ac_symbol));
        t = lista;
        lista->oper_id = operand_id;  
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
        *ptr_bufInitial = *args;
        args++;
        ptr_bufInitial++;
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
void replace(char *ptr_ret, char *old, char *new){
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
