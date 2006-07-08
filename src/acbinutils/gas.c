/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      gas.c
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
 * @brief     GNU assembler related code (implementation)
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdio.h>
#include "utils.h"
#include "gas.h"


int CreateEncodingFunc(const char *encfunc_filename)
{ /* only works with 32-bit max */
  FILE *output;

  if ((output = fopen(encfunc_filename, "w")) == NULL) 
    return 0;
  
  /* instruction format type switch */
  fprintf(output, "%sswitch(insn_type) {\n", IND1);
 
  ac_dec_format *pfrm = format_ins_list;
  ac_dec_field *pf = NULL;

  int i=0;
  while (pfrm != NULL) {
    fprintf(output, "%scase %i:\n", IND2, i);

    /* field number switch */
    fprintf(output, "%sswitch (field_id) {\n", IND3);
   
    pf = pfrm->fields;
    int j=0;
    while (pf != NULL) {
      unsigned long mask = 
              (0xffffffff >> 
                 (32 - pfrm->size + pf->first_bit+1 - pf->size)) 
            & (0xffffffff << 
                 (pfrm->size - (pf->first_bit+1))); 

      fprintf(output, "%scase %i:\n", IND4, j);
      fprintf(output, "%s*image &= 0x%lx;\n", IND5, ~mask); 
      fprintf(output, "%s*image |= ((value << %i) & 0x%lx);\n", IND5, pfrm->size - (pf->first_bit+1), mask);
      fprintf(output, "%sbreak;\n", IND5);
      
      pf = pf->next;
      j++;
    }
    fprintf(output, "%s}\n", IND3);
 
    fprintf(output, "\n%sbreak;\n", IND3);
    pfrm = pfrm->next;
    i++;
  }
  fprintf(output, "\n%s}\n", IND1);
 
  fprintf(output, "%sreturn 0;", IND1);

  fclose(output);
  return 1;
}



int CreateGetFieldSizeFunc(const char *getfsz_filename)
{
  FILE *output;

  if ((output = fopen(getfsz_filename, "w")) == NULL) 
    return 0;

  /* instruction type switch */
  fprintf(output, "%sswitch(insn_fmt) {\n", IND1);
 
  ac_dec_format *pfrm = format_ins_list;
  ac_dec_field *pf = NULL;

  int i=0;
  while (pfrm != NULL) {
    fprintf(output, "%scase %i:\n", IND2, i);

    /* field number switch */
    fprintf(output, "%sswitch (field_id) {\n", IND3);    
    
    pf = pfrm->fields;
    int j=0;
    while (pf != NULL) {
      fprintf(output, "%scase %i:\n", IND4, j);
      fprintf(output, "%sreturn %i;\n\n", IND4, pf->size);
      
      pf = pf->next;
      j++;
    }
    fprintf(output, "%s}\n", IND3);

    fprintf(output, "\n%sbreak;\n", IND3);
    pfrm = pfrm->next;
    i++;
  }
  fprintf(output, "\n%s}\n", IND1);

  fprintf(output, "%sreturn 0;", IND1);

  fclose(output);
  return 1;
}


int CreateGetInsnSizeFunc(const char *insnsz_filename)
{
  FILE *output;

  if ((output = fopen(insnsz_filename, "w")) == NULL) 
    return 0;
  
  /* instruction type switch */
  fprintf(output, "%sswitch(insn_fmt) {\n", IND1);
 
  ac_dec_format *pfrm = format_ins_list;

  int i=0;
  while (pfrm != NULL) {
    fprintf(output, "%scase %i:\n", IND2, i);

    fprintf(output, "%sreturn %d;\n", IND3, pfrm->size);

    pfrm = pfrm->next;
    i++;
  }
  fprintf(output, "\n%s}\n", IND1);

  fprintf(output, "%sreturn 0;", IND1);

  fclose(output);
  return 1;
}
