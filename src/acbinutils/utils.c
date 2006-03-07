/* ex: set tabstop=2 expandtab: */

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
