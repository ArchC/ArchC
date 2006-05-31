/* 
    ArchC disassembler generator - Copyright (C) 2002-2006  The ArchC Team

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
/* xxxxx-dis.c: The ArchC disassembler generator.       */
/* Author: Alexandre Keunecke I. de Mendonça            */
/*         Felipe Guimarães Carvalho                    */
/*         Max Ruben de Oliveira Schultz                */
/* Date: 08-03-2005                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Systems Design Automation Lab (LAPS)                 */
/* Federal University of Santa Catarina (UFSC)          */
/* http://www.laps.inf.ufsc.br                          */
/********************************************************/

#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opintl.h"
#include `"opcode/'___arch_name___`.h"'

/*----------------------------------------------------------------------------*/

#define `MAX_FORMAT_SIZE '___max_format_size___
#define `VARIABLE_FORMAT_SIZE '___variable_format_size___
#define BI_NONE           0
#define BI_MD_REL_BIT    (1 << 3)
#define BI_MD_AL_BIT     (1 << 6)

/*----------------------------------------------------------------------------*/

// Disassembler generation definitions...

/*
  Linked list with all the operands of the isntruction.
  field_id -> position of field in the instruction, later used to know about the size of field, (examples: '1' or '1+2')
  type -> type of field e.g. reg, addr, immm, exp
*/
typedef struct _ac_symbol {
   char *field_id;
   char *type;
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

/*
   Addend variables - set only by get_addend()
   Don't ever change these variables elsewhere!
*/
static int valr = 0;   // value used to return R addends
static int vala = 0;   // value used to return A addends


/* assuming max input = 64 */
static int log_table[] = {  0 /*invalid*/,  0, 1, 1,
                            2 /* log 4 */,  2, 2, 2,
                            3 /* log 8 */,  3, 3, 3,
                            3,              3, 3, 3,
                            4 /* log 16 */, 4, 4, 4  };


/*-----------------------------------------------------------------------------------*/

static int disassemble (bfd_vma memaddr, struct disassemble_info *info, unsigned long insn, int insn_size);

int `print_insn_'___arch_name___` (bfd_vma memaddr, struct disassemble_info * info);'

static unsigned long get_field_size(unsigned long insn_fmt, int field_id);

static unsigned long get_insn_size(unsigned long insn_fmt);

ac_symbol* parse(char *args);

static unsigned int get_builtin_marker(char *s);

static void get_addend(char addend_type, char **st);

void replace(char *str, char *old, char *new);

/*------------------------------------------------------------------------------------*/


#define FETCH_DATA(info, addr) \
  ((addr) <= ((struct private *)(info->private_data))->max_fetched \
   ? 1 : fetch_data ((info), (addr)))

static int
fetch_data (struct disassemble_info *info, bfd_byte *addr)
{
  int status;
  struct private *priv = (struct private *) info->private_data;
  bfd_vma start = priv->insn_start + (priv->max_fetched - priv->the_buffer);

 

  status = (*info->read_memory_func) (start,
				      priv->max_fetched,
				      addr - priv->max_fetched,
				      info);
  /* verify (status != 5) because for architecture with variable format size are read 8, 16, 24 and 32 bits of memory and in the last instruction error can occur */
  if ((status != 0) && (status != 5))
    {
      (*info->memory_error_func) (status, start, info);
      longjmp (priv->bailout, 1);
    }
  else
    priv->max_fetched = addr;

  return 1;
}

static unsigned long get_field_size(unsigned long insn_fmt, int field_id)
{
___fieldsize_function___
}

static unsigned long get_insn_size(unsigned long insn_fmt)
{
___insnsize_function___
}


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
   
   //Find the mnemonic of instruction in opcodes table
   while (op->mnemonic){
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
            unsigned long begining = 0;

            char *ptfield_id = malloc(sizeof(char)*50);
            ptfield_id = acs->field_id;
            unsigned long size = 0;
            unsigned int last_field_id = 0;
            while (*ptfield_id != '\0') {
               if ((*ptfield_id >= '0') && (*ptfield_id <= '9')) {
                 size = size + get_field_size(op->format_id, atoi(ptfield_id));
                 last_field_id = (unsigned)atoi(ptfield_id);
               }
               ptfield_id++;
            }

            unsigned int i = 0;
            for (i=0; i<=last_field_id; i++){
               begining += get_field_size(op->format_id, i);
            }

            unsigned long dmask = 0x00;
            for (i=0; i<size; i++){
               dmask = (dmask << 1) + 1;
            }

            dmask = dmask << (FORMAT_SIZE - begining);//operand dmask
            unsigned long value = dmask & insn;
            value = value >> (FORMAT_SIZE - begining); //dislocates masks to see it which reg ex: [0,31]
            int cont = 0;
            int in = 0;
            char symbolaux[50];



            /* Search the register table, looking value (taken from the dmask) is a register and finds the name of the register and prints it as operating of instruction, if not found, will be an immediate field or another thing it prints the value directly,*/

            while (cont < num_symbols){
               if ((strcmp(acs->type, symbols->cspec) == 0) && (value == symbols->value)){
                  //operand find
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
            }else{//not operand find, it can be: exp, immm, addr
               //verifies if has modifiers and it records in vala,and valr
               unsigned int bi_type = get_builtin_marker(acs->type);

               int align = (bi_type & BI_MD_AL_BIT) ? 1 : 0;
               int pcrel = (bi_type & BI_MD_REL_BIT) ? 1 : 0;

               if (align){ //Modifier A
                  value <<= (vala ? log_table[vala] : log_table[FORMAT_SIZE / 8]);
               }
               if (pcrel){ //Modifier R
                  if (value & (1<<(size-1))){
                     value |= ((0xFFFFFFFF >> (32 - size)) & (0xFFFFFFFF << (size-1)));
                     value = (((0xFFFFFFFF >> (32 - size)) & ~value) + 1);
                     value = memaddr - value + valr;              
                  } 
                  else
                     value += memaddr + valr;
               }
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
   }
   (*info->fprintf_func) (info->stream,"%s ",bufFinal);
   return  FORMAT_SIZE / 8;
}


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

   if (setjmp (priv.bailout) != 0)
      /* Error return.  */
      return -1;

   acasm_opcode *op = (acasm_opcode *) opcodes;

   /* Verify if architecture has variable format size (CISC) ou no (RISC) */
   if (VARIABLE_FORMAT_SIZE == 0) {
      /*RISC - read fix length bits of the memory */
      FETCH_DATA (info, buffer + 4);
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
         while ((op->mnemonic) && (op->dmask != 0)) {
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

   return disassemble(memaddr, info, insnfound, sizeinsn);
}


ac_symbol* parse(char *args){
   char *ptr_bufInitial = bufInitial;
   ac_symbol *lista = NULL;
   ac_symbol *temp;
   ac_symbol *t;
   t = NULL;
   temp = NULL;
   int i = 0;

   while (*args != '\0'){
      char buf[50];
      char *ptr_buf = buf;
      char field_aux[50];
      char *ptr_field_aux = field_aux;

      /*
        '%' is the start of a well-formed sentence. It goes like this:
        '%'<conversion specifier>':'<insn field ID>':<information from ld>:
        ' %imm:5:0:,%reg:3:0:,%reg:1:0:
      */
      if (*args == '%'){
         args++;
         while (*args != ':'){
            *ptr_buf = *args;
            *ptr_bufInitial = *args;
            ptr_bufInitial++;
            ptr_buf++;
            args++;
         }

         *ptr_buf = '\0';
         args++;

         char *arg_start = args;
         while ((*args >= '0') && (*args <= '9'))
            args++;

         *ptr_field_aux = *arg_start;
         ptr_field_aux++;
         args++;

         while ((*args != ':') && (*args != '+'))
            args++;
         
         if (*args == ':') {
           args++;
         }
         else if (*args == '+') {
           *ptr_field_aux = *args;
           ptr_field_aux++;
           args++;
           *ptr_field_aux = *args;
           ptr_field_aux++;
           args++;
           args++;
           while (*args != ':')
             args++;
           args++; 
         }

         *ptr_field_aux = '\0';
         ptr_field_aux++;

         char *to2 = malloc(sizeof(char)*50);
         strcpy(to2, field_aux);

         char *to = malloc(sizeof(char)*50);
         strcpy(to, buf);
         
         if (i > 0){
            temp = (ac_symbol *) malloc(sizeof(ac_symbol));
            temp->field_id = to2;
            temp->type = to;
            temp->next = NULL;
            lista->next = temp;
            lista = lista->next;
         }
         else{
            lista = (ac_symbol *) malloc(sizeof(ac_symbol));
            t = lista;
            lista->field_id = to2;
            lista->type = to;
            lista->next = NULL;
         }
         i++;
      }else{  //if (*args=='%')
         if (*args == '\\'){
            args++;
            args++;
            while ((*args>64 && *args<91) || (*args>96 && *args<123))
               args++;
         }else{
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
 Bases:
  'exp'
	'addr'
	'imm'

 Modifiers:
  'H'[n][c][u|s] -> n -> get the n-high bit -- u or s (unsigned or unsigned) -- c - add carry from lower bits
  'L'[n][u|s] -> get the n-low bits
	'R'[n][b] -> backward, n -> add n to PC
	'A'[n][u|s] -> align in a paragraph of n-bits

 st : null-terminated string with the conversion specifier
*/
static unsigned int get_builtin_marker(char *st){
   valr  = 0;
   vala  = 0;

   unsigned int ret_val = BI_NONE;

   if (*st == 'e' && *(st+1) == 'x' && *(st+2) == 'p'){
      st += 3;
   }
   else if (*st == 'a' && *(st+1) == 'd' && *(st+2) == 'd' && *(st+3) == 'r'){
      st += 4;
   }
   else if (*st == 'i' && *(st+1) == 'm' && *(st+2) == 'm'){
      st += 3;
   }
   else
      return BI_NONE;

   while (*st != '\0'){
      switch (*st){
         case 'A':
            ret_val |= BI_MD_AL_BIT;
            get_addend('A', &st);
            break;
         case 'R':
            ret_val |= BI_MD_REL_BIT;
            get_addend('R', &st);
           break;
      }
      st++;
   }

   return ret_val;
}

/*----------------------------------------------------------------------------*/

/*
  Parse and get the modifier's addend
  Only called by get_builtin_marker()

  addend_type: ''A' or 'R'

  st : string pointing to the addend type. It will be set to the next char
  after the addend
*/
static void get_addend(char addend_type, char **st){
   char *sl = *st;
   sl++;

   while (*sl >= '0' && *sl <= '9')
      sl++;

   if ((sl-1) != *st){
      char savec = *sl;
      *sl = '\0';

      if (addend_type == 'A')
         vala = atoi((*st)+1);
      else // 'R'
         valr = atoi((*st)+1);

      *sl = savec;
   }

   switch (addend_type){ 
      case 'R': // [b]
         if (*sl == 'b'){
           sl++;
           valr = valr * (-1);
         }
         break;
      case 'A': // [u|s]
         if (*sl == 's'){
           sl++;
         }
         else if (*sl == 'u'){
           sl++;
         }
         break;
   }

   *st = sl-1;
}


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
         while(*old == *str){
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
