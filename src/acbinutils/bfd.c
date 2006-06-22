/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      bfd.c
 * @author    Alexandro Baldassin (UNICAMP)
 *            Daniel Casarotto (UFSC)
 *            Max Schultz (UFSC)
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 *            System Design Automation Lab (LAPS)
 *            INE-UFSC
 *            http://www.laps.inf.ufsc.br/
 * 
 * @version   1.0
 * @date      Thu, 01 Jun 2006 14:28:06 -0300
 * 
 * @brief     BFD library related code (implementation)
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "bfd.h"

// Linker generation definitions...

typedef struct _ac_relocation_type {
  char *name;
  unsigned int id;
  unsigned int fields;
  unsigned int format_id;
  operand_modifier mod_id;
//  unsigned rightshift;
//  unsigned reloc_size;
//  unsigned bitsize;
//  unsigned bitpos;
//  unsigned mask;
//  unsigned pc_relative;
//  unsigned uses_carry;
  struct _ac_relocation_type *next;
} ac_relocation_type;

static ac_relocation_type* find_relocation_by_id(unsigned id); 
static void process_instruction_relocation(ac_asm_insn *asml);
static ac_relocation_type* find_relocation(ac_relocation_type* reloc);
static int compare_relocs(ac_relocation_type* first, ac_relocation_type* second);



static ac_relocation_type *relocation_types_list = NULL;  


void create_relocation_list()
{
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();

  while (asml != NULL) {
    if (asml->insn != NULL) { /* native instructions */
      process_instruction_relocation(asml);      
    }
    asml = asml->next;
  }

}

/*
  Relocation IDs - include/elf/<arch>.h
*/
int CreateRelocIds(const char *relocid_filename) 
{
  FILE *output;

  if ((output = fopen(relocid_filename, "w")) == NULL) 
    return 0;

  fprintf(output, "%sRELOC_NUMBER (R_%s_NONE, 0)\n", IND1, get_arch_name());

  unsigned reloc_id = 1;
  ac_relocation_type* relocation = find_relocation_by_id(reloc_id);
  while (relocation) {
    fprintf (output, "%sRELOC_NUMBER (%s, %d)\n", IND1, relocation->name, relocation->id);
    reloc_id++;
    relocation = find_relocation_by_id(reloc_id);
  }
  
  // Generic data relocations
  fprintf(output, "%sRELOC_NUMBER (R_%s_8,  %d)\n", IND1, get_arch_name(), reloc_id);
  fprintf(output, "%sRELOC_NUMBER (R_%s_16, %d)\n", IND1, get_arch_name(), reloc_id + 1);
  fprintf(output, "%sRELOC_NUMBER (R_%s_32, %d)\n", IND1, get_arch_name(), reloc_id + 2);
  
  fprintf(output, "%sRELOC_NUMBER (R_%s_REL8,  %d)\n", IND1, get_arch_name(), reloc_id + 3);
  fprintf(output, "%sRELOC_NUMBER (R_%s_REL16, %d)\n", IND1, get_arch_name(), reloc_id + 4);
  fprintf(output, "%sRELOC_NUMBER (R_%s_REL32, %d)", IND1, get_arch_name(), reloc_id + 5);
  
  fclose(output);
  return 1;
}

/*
  Relocation HOWTO structure - bfd/elf32-<arch>.c

  Fields of the original BFD structure (reloc_howto_struct)
  
  . unsigned int type;
  . unsigned int rightshift;
  . int size;
  . unsigned int bitsize;
  . bfd_boolean pc_relative;
  . unsigned int bitpos;
  . enum complain_overflow complain_on_overflow;
  . bfd_reloc_status_type (*special_function)
    (bfd *, arelent *, struct bfd_symbol *, void *, asection *,
     bfd *, char **);
  . char *name;
  . bfd_boolean partial_inplace;
  . bfd_vma src_mask;
  . bfd_vma dst_mask;
  . bfd_boolean pcrel_offset;

  Mapping from 'ac_relocation_type' to 'reloc_howto_struct'

  id        --> type
  fields    --> rightshift
  format_id --> bitsize
  mod_id    --> bitpos

*/
int CreateRelocHowto(const char *reloc_howto_filename) 
{
  FILE *output;

  if ((output = fopen(reloc_howto_filename, "w")) == NULL) 
    return 0;

  fprintf(output, "%sHOWTO (R_%s_NONE, 0, 0, 0, FALSE, 0, complain_overflow_dont, bfd_elf_archc_reloc, \"R_%s_NONE\", FALSE, 0, 0, FALSE),\n", IND1, get_arch_name(), get_arch_name()); 
  
  unsigned reloc_id = 1;
  ac_relocation_type *relocation = find_relocation_by_id(reloc_id);
  while (relocation) {
//    char relocation_function[256];
//    sprintf(relocation_function, "_bfd_%s_elf_carry_reloc", get_arch_name());
    
    fprintf (output, "%sHOWTO (%s, %d, %d, %d, %s, %d, %s, %s, \"%s\", %s, 0x%x, 0x%x, %s),\n",
      IND1,
      relocation->name,       /* type */
      relocation->fields,     /* rightshift */
//      relocation->rightshift,
      0, /* not used */       /* size */
      relocation->format_id,  /* bitsize */
//      relocation->bitsize,
      "FALSE", /* not used */ /* pc-relative */
      relocation->mod_id,     /* bitpos */
//      relocation->bitpos,
      "complain_overflow_dont", /* not used */
      "bfd_elf_archc_reloc",    /* always this one */
      relocation->name,       /* name */
      "TRUE", /* not used */  /* partial_inplace */
      0, /* not used */       /* src mask */
      0, /* not used */       /* dst mask */
      "TRUE" /* not used */   /* pcrel_offset */
    );
    reloc_id++;
    relocation = find_relocation_by_id(reloc_id);
  }
  // Generic data relocations
  fprintf(output, "%sHOWTO (R_%s_8,  0, 8,  8, FALSE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_8\",    FALSE, 0, 0x000000ff, TRUE),\n", IND1, get_arch_name(), get_arch_name());
  fprintf(output, "%sHOWTO (R_%s_16, 0, 16, 16, FALSE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_16\",   FALSE, 0, 0x0000ffff, TRUE),\n", IND1, get_arch_name(), get_arch_name());
  fprintf(output, "%sHOWTO (R_%s_32, 0, 32, 32, FALSE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_32\",   FALSE, 0, 0xffffffff, TRUE),\n", IND1, get_arch_name(), get_arch_name());

  fprintf(output, "%sHOWTO (R_%s_REL8,  0, 8,  8, TRUE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_REL8\",    FALSE, 0, 0x000000ff, TRUE),\n", IND1, get_arch_name(), get_arch_name());
  fprintf(output, "%sHOWTO (R_%s_REL16, 0, 16, 16, TRUE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_REL16\",   FALSE, 0, 0x0000ffff, TRUE),\n", IND1, get_arch_name(), get_arch_name());
  fprintf(output, "%sHOWTO (R_%s_REL32, 0, 32, 32, TRUE, 0, complain_overflow_bitfield, bfd_elf_archc_reloc,  \"R_%s_REL32\",   FALSE, 0, 0xffffffff, TRUE),\n", IND1, get_arch_name(), get_arch_name());

  fclose(output);
  return 1;
}


/*
  Relocation map - bfd/elf32-<arch>.c
*/
int CreateRelocMap(const char *relocmap_filename)
{
  FILE *output;

  if ((output = fopen(relocmap_filename, "w")) == NULL) 
    return 0;
  
  fprintf(output,"%s{ BFD_RELOC_NONE, R_%s_NONE },\n",IND1, get_arch_name());

  unsigned reloc_id = 1;
  ac_relocation_type *relocation = find_relocation_by_id(reloc_id);
  while (relocation) {
    reloc_id++;
    ac_relocation_type* next_relocation = find_relocation_by_id(reloc_id);
    fprintf(output,"%s{ %d, %s },\n", IND1, relocation->id, relocation->name);
    relocation = next_relocation;
  }

  // Generic data relocations
  fprintf(output,"%s{ BFD_GENERIC_8,  R_%s_8  },\n",IND1, get_arch_name());  
  fprintf(output,"%s{ BFD_GENERIC_16, R_%s_16 },\n",IND1, get_arch_name());  
  fprintf(output,"%s{ BFD_GENERIC_32, R_%s_32 },\n",IND1, get_arch_name());

  fprintf(output,"%s{ BFD_GENERIC_REL8,  R_%s_REL8  },\n",IND1, get_arch_name());  
  fprintf(output,"%s{ BFD_GENERIC_REL16, R_%s_REL16 },\n",IND1, get_arch_name());  
  fprintf(output,"%s{ BFD_GENERIC_REL32, R_%s_REL32 }\n", IND1, get_arch_name());  

  fclose(output);
  return 1;
}



static ac_relocation_type* find_relocation_by_id(unsigned id) 
{
  ac_relocation_type* relocation = relocation_types_list;
  while (relocation) {
    if (relocation->id == id)
      return relocation;
    relocation = relocation->next;
  }
  
  return NULL;
}


static void process_instruction_relocation(ac_asm_insn *asml)
{
  ac_operand_list *opP = asml->operands;

  while (opP != NULL) { /* for each operand */
  
    if (opP->type != op_addr && opP->type != op_exp) {
       opP = opP->next;
       continue;
    }
     
    ac_asm_insn_field *fieldP = opP->fields;
    ac_relocation_type* reloc = (ac_relocation_type*) malloc(sizeof(ac_relocation_type));
    reloc->fields = 0;

    while (fieldP != NULL) { /* encode field */
    /* field id is encoded within bits in a long long variable 
      field_id 0 -> ... 0000 0001
      field_id 2 -> ... 0000 0100
      ids 3 e 4  -> ... 0001 1000
    */
      reloc->fields |= 1 << fieldP->id;

      fieldP = fieldP->next;
    }
    reloc->format_id = get_format_id(asml->insn->format);

    reloc->mod_id = opP->modifier.type;



    ac_relocation_type* reloc_found = find_relocation(reloc);

    if (reloc_found == NULL) {
      reloc->id = relocation_types_list ? relocation_types_list->id + 1 : 1;
      reloc->name = (char*) malloc(sizeof(char)*128);
      sprintf(reloc->name, "R_%s_ID%d_F%XFt%uM%d", get_arch_name(), reloc->id, reloc->fields, reloc->format_id, reloc->mod_id); 
      reloc->next = relocation_types_list;
      relocation_types_list = reloc;
      opP->reloc_id = reloc->id;
    } 
    else { /* reloc found */
      opP->reloc_id = reloc_found->id;          
      free(reloc);
    }

    opP = opP->next;
  }



     /* Get the sum of all fields in bits (from left to right)
      */
//    unsigned int fields_bit_size = 0;
//    for (; fieldP != NULL; fieldP = fieldP->next) 
//      fields_bit_size += fieldP->size; 
    
//    fieldP = opP->fields;
//    while (fieldP != NULL) { /* for each field */

//      fields_bit_size -= fieldP->size;
       
      /* fill in the relocation fields */

      /*
       * rightshift 
       * - number of bits the reloc value will be shifted to the right
       *
       *   if the operand has multiple fields assigned to, then the 
       *   left fields must be shifted to the right
       */
//      unsigned rightshift = 0+fields_bit_size;

      /*
       * reloc_size
       * - size of the relocation field
       *   it is the size of the instruction
       *
       *   NOTE: Since we've made our own relocation routines, this value
       *   is being used as the bit size
       */
      //unsigned reloc_size = log_table[get_insn_size(asml)/8];
//      unsigned reloc_size = get_insn_size(asml);

      /*
       * bitsize
       * - size (in bits) of the field to be relocated
       */
//      unsigned bitsize = fieldP->size;


      /*
       * bitpos
       * - bit position of the relocation value in the destination
       *   the relocation is shifted left by the number of bits 
       *   specified in this variable
       */
//      unsigned bitpos = get_insn_size(asml) - (fieldP->first_bit+1);

      /*
       * mask
       * - selects which parts of the relocation field is inserted
       *   into the binary image
       */
//      unsigned long mask = 0;
//      unsigned i;
//      for (i=0; i < get_insn_size(asml); i++)
//        mask |= (1 << i);

      
//      mask >>= (get_insn_size(asml) - fieldP->size);

       /*
        * uses_carry
        * - if the carry of the lower part affect the high part of
        *   a field, then this flag should be 1
        */
//      unsigned uses_carry = 0;

      /*
       * is_pcrel
       * - 1 if the field is pc-relative
       */
//      unsigned is_pcrel = 0;

      /*
       * has_high
       * - 1 if the operand has a high modifier assigned
       *
       */
//      unsigned has_high = 0;

       /*
        * now apply the modifiers to the relocation fields
        */
//      ac_modifier_list *modP = opP->modifiers;
//      while (modP != NULL) {

//        switch (modP->type) {
//          case mod_low:
//            if (modP->addend >= 0 && modP->addend <= fieldP->size) {
//              mask >>= (fieldP->size - modP->addend);
//              bitsize = modP->addend;
//            }
            /* signed / unsigned - not being used! */
//            break;
            
//          case mod_high:
//            if (modP->addend >= 0 && modP->addend <= fieldP->size) {
//              rightshift += get_insn_size(asml) - modP->addend;
//              mask >>= (fieldP->size - modP->addend);
//              bitsize = modP->addend;
//            }
//            else
//              rightshift += get_insn_size(asml) - fieldP->size;

//            uses_carry = modP->carry;
//            has_high = 1;
            
            /* signed / unsigned - not being used! */            
//            break;
            
//          case mod_aligned:
//              if (modP->addend == -1)
//                 modP->addend = get_arch_size()/8;
//              rightshift += log_table[modP->addend];
//            break;
            
//          case mod_pcrel:
//            is_pcrel = 1;
//            break;
            
//          case mod_pcrelext:
//            is_pcrel = 1;
//            break;
//        }

//        modP = modP->next;
//      }
   
//      mask <<= bitpos;
      
//      ac_relocation_type* reloc = (ac_relocation_type*) malloc(sizeof(ac_relocation_type));
//      reloc->rightshift = rightshift;
//      reloc->reloc_size = reloc_size;
//      reloc->bitsize = bitsize;
//      reloc->pc_relative = is_pcrel;
//      reloc->bitpos = bitpos;
//      reloc->mask = mask;
//      reloc->uses_carry = uses_carry;

//      ac_relocation_type* reloc_found = find_relocation(reloc);

//      if (!reloc_found) {
//        reloc->id = relocation_types_list ? relocation_types_list->id + 1 : 1;
//        reloc->name = (char*) malloc(sizeof(char)*128);
//        sprintf(reloc->name, "R_%s_%d_%s%s%d%s", get_arch_name(), reloc->id, (is_pcrel ? "REL" : "") , (has_high ? "HI" : "LO"), bitsize, uses_carry ? "_CARRY" : "");
        
//        reloc->next = relocation_types_list;
//        relocation_types_list = reloc;
//        fieldP->reloc_id = reloc->id;
//      } 
//      else {
//        fieldP->reloc_id = reloc_found->id;          
//        free(reloc);
//      }

//      fieldP = fieldP->next;
//    }

//    opP = opP->next;
//  }
  
}



static ac_relocation_type* find_relocation(ac_relocation_type* reloc)
{
  ac_relocation_type* relocation = relocation_types_list;
  
  while (relocation != NULL) {
    if (compare_relocs(reloc,relocation))
      return relocation;
    relocation = relocation->next;
  }
  
  return NULL;
}

static int compare_relocs(ac_relocation_type* first, ac_relocation_type* second)
{
  return first->fields    == second->fields &&
         first->format_id == second->format_id &&
         first->mod_id    == second->mod_id;
/*
  return first->rightshift == second->rightshift &&
         first->reloc_size == second->reloc_size &&
         first->bitsize == second->bitsize &&
         first->bitpos == second->bitpos &&
         first->mask == second->mask &&
         first->pc_relative == second->pc_relative &&
         first->uses_carry == second->uses_carry;
*/
}
