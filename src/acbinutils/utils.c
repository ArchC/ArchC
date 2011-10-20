/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      utils.c
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
 * @brief     Utilities routines for the acbinutils module
 *            (implementation)
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include "utils.h"


static char *arch_name = NULL; /* name of the architecture */

/* assuming max input = 64 */
int log_table[] = {  0 /*invalid*/,  0, 1, 1, 
                     2 /* log 4 */,  2, 2, 2, 
                     3 /* log 8 */,  3, 3, 3,
                     3,              3, 3, 3,
                     4 /* log 16 */, 4, 4, 4 };

/* General internal error handling function */
void internal_error() {
  printf("Internal Error. Contact the ArchC Team.\n");
  exit(-1);
}

/*
 * Returns the format id of the string fname. (-1 if not found!)
 */
int get_format_id(char *fname)
{
  ac_dec_format *pfrm = format_ins_list;
  unsigned int formatid = 0;
  while ((pfrm != NULL) && strcmp(fname, pfrm->name)) {
    formatid++;
    pfrm = pfrm->next;
  }
  if (pfrm == NULL) return -1;
  return formatid;
}



/*
 * Returns the size of the architecture word (in bits)
 * It uses the ac_wordsize currently
 * Note that it only support architectures with, at most, 32-bit words
 */
unsigned int get_arch_size() 
{
  extern int wordsize;
  return wordsize ? wordsize : 32;
}

/*
 * Returns the size of an instruction format (in bits)
 */
unsigned int get_insn_size(ac_asm_insn *insn) 
{
  extern ac_dec_format *format_ins_list;
  ac_dec_format *pfrm = format_ins_list;
    
  if (insn->insn != NULL) { 

    while ((pfrm != NULL) && strcmp(insn->insn->format, pfrm->name)) 
        pfrm = pfrm->next;

    if (pfrm == NULL) internal_error();
    else return pfrm->size;
  }
  else internal_error();
}

void set_arch_name(char *str) 
{
  if (arch_name != NULL) {
    free(arch_name);
    arch_name = NULL;
  }

  if (str == NULL)
    return;
  
  arch_name = (char *) malloc(strlen(str)+1);
  strcpy(arch_name, str);
}

char *get_arch_name()
{
  return arch_name;
}

/*
 * Returns the max format size of the architecture word (in bits)
 */
unsigned int get_max_format_size() 
{
  ac_dec_format *pfrm = format_ins_list;
  int max_size = 0;
             
  while (pfrm != NULL) {
    if (pfrm->size > max_size)
      max_size = pfrm->size;
    pfrm = pfrm->next;
  }

  return max_size;
}

/*
 * Returns if the architecture has variable formats (1) or no (0)
 */
unsigned int get_variable_format_size() 
{
  ac_dec_format *pfrm = format_ins_list;

  int variable_format_size = 0;
  int size = pfrm->size;
               
  while (pfrm != NULL) {
    if (pfrm->size != size) {
      variable_format_size = 1;
      break;
    }
    pfrm = pfrm->next;
  }

  return variable_format_size;
}

/*
 * Returns the format number of fields
 */
unsigned int get_format_number_fields(ac_dec_format *format) 
{
  ac_dec_field *fields = format->fields;

  unsigned int number_fields = 0;
               
  while (fields != NULL) {
    number_fields++;

    fields = fields->next;
  }

  return number_fields;
}

/*
 * Returns the maximum number of fields among all the formats
 */
unsigned int get_max_number_fields() 
{
  ac_dec_format *pfrm = format_ins_list;
  unsigned int max_number_fields = 0;

  while (pfrm != NULL) {
    unsigned int nt = get_format_number_fields(pfrm);
    if (nt > max_number_fields) 
      max_number_fields = nt;

    pfrm = pfrm->next;
  }  

  return max_number_fields;
}


/* field is encoded within bits in a 32-bit variable 
 *   field_id 0 -> ... 0000 0001
 *   field_id 2 -> ... 0000 0100
 *   ids 3 e 4  -> ... 0001 1000
 */
unsigned int encode_fields(ac_asm_insn_field *fieldP)
{
  unsigned int encoding = 0;

  while (fieldP != NULL) { /* encode field */
    encoding |= 1 << fieldP->id;

    fieldP = fieldP->next;
  }
  return encoding;
}

unsigned int encode_fields_positions(ac_asm_insn_field *fieldP, unsigned insn_size)
{
  unsigned int encoding = 0, endbit = 0, startbit = 0;

  while (fieldP != NULL) { /* encode field */
    startbit = insn_size - fieldP->first_bit - 1;
    endbit = startbit + fieldP->size - 1;
    encoding |= ((1 << startbit) | (1 << endbit));
    fieldP = fieldP->next;
  }
  return encoding;
}
