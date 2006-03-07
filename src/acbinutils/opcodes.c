/* ex: set tabstop=2 expandtab: */

#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "opcodes.h"

/*
 * module function prototypes
 */
static void create_operand_string(ac_asm_insn *insn, char **output);
static unsigned int encode_insn_field(unsigned int field_value, unsigned insn_size, unsigned fbit, unsigned fsize);



/* 
   Opcode table generation 

  This table is of type acasm_opcode with the following structure:

  . const char *mnemonic
    The mnemonic string (cannot be NULL)

  . const char *args
    A string with the syntax of the operands (may be NULL)
    The following format is used:
      args ::=  [literal_chars] (operand_tag literal_chars)*
      operand_tag ::= '%' format_specifier ':' field_id ':'

      format_specifier: may be a user defined or a built-in specifier (a string)
      literal_chars: a sequence of valid chars which the assembler needs to match literally
      field_id: an integer representing the field of this operand in the instruction format
 
  . unsigned long image
    The base binary image of this instruction

  . unsigned long format_id
    The format identifier. 0 is the first format in the list, 1 is the second, and so on

  . unsigned long pseudo_idx
    An index to the first instruction to be executed in case this is a pseudo instruction

  . unsigned long counter
    (optional) Used to count the number of instruction assembled

*/
int CreateOpcodeTable(const char *table_filename)
{
  char *strP;
  FILE *output;

  if ((output = fopen(table_filename, "w")) == NULL) 
    return 0;

  /* get the instruction list from parser */
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();

  long pseudo_idx = 1;  /* index to an instruction in the pseudo_instr table */  
  while (asml != NULL) {

    /* mnemonic */
    /*----------------------------------------------------------------*/
    fprintf(output, "%s{\"%s\",\t", IND1, asml->mnemonic);
    /*----------------------------------------------------------------*/

    /* args string */
    /*----------------------------------------------------------------*/
    char *buffer = (char *)malloc(100);
    create_operand_string(asml, &buffer);
    fprintf(output, "\"%s\",\t", buffer);
    free(buffer);
    /*----------------------------------------------------------------*/
    
    /* base image */
    /*----------------------------------------------------------------*/
    unsigned long base_image = 0x00;
    unsigned long format_id = 99;     /* a pseudo_instr has a 99 format id */

    ac_dec_format *pfrm = format_ins_list;
    
    if (asml->insn != NULL) { /* native instructions */

      /* get the format of this instruction. 0 is the first */
      format_id = 0;
      while ((pfrm != NULL) && strcmp(asml->insn->format, pfrm->name)) {
          format_id++; 
          pfrm = pfrm->next;
      }

      if (pfrm == NULL) internal_error();
      else {
          base_image = 0x00;
        /* for each decoding field, finds its place in the format and encode it in 
        the base_image variable */
          {
             ac_dec_list *pdl = asml->insn->dec_list;
             while (pdl != NULL) {       
                ac_dec_field *pdf = pfrm->fields;
                while ((pdf != NULL) && strcmp(pdl->name, pdf->name)) pdf = pdf->next;
         
                if (pdf == NULL) internal_error();
                else base_image |= encode_insn_field(pdl->value, pfrm->size, pdf->first_bit, pdf->size);
                 
                pdl = pdl->next;         
             }  
             ac_const_field_list *cfP = asml->const_fields;
             while (cfP != NULL) {
                
               base_image |= encode_insn_field(cfP->value, pfrm->size, cfP->field.first_bit, cfP->field.size);
               cfP = cfP->next;
             }
          }

      }      
    }
    fprintf(output, "0x%08X,\t", base_image);      
    /*----------------------------------------------------------------*/
  
    /* format id */
    /*----------------------------------------------------------------*/
    fprintf(output, "%d,\t", format_id);
    /*----------------------------------------------------------------*/

    /* if a pseudo, saves the pseudo-instr table instruction index */
    /*----------------------------------------------------------------*/
    if (asml->pseudolist != NULL) {
      fprintf(output, "%d", pseudo_idx);
      pseudo_idx += asml->num_pseudo+1;
    }
    else
      fprintf(output, "0");
    /*----------------------------------------------------------------*/

    /***
     Optional statistic field
     ***/
    fprintf(output, ",\t0");
     

    fprintf(output, "},\n"); 

    asml = asml->next;
  } /* end-of-while */

  fclose(output);
  return 1; 
}

/*
  User defined symbol table generation

  TODO: describe the acasm_symbol structure

*/
int CreateAsmSymbolTable(const char *symtab_filename) 
{
  FILE *output;

  if ((output = fopen(symtab_filename, "w")) == NULL) 
    return 0;
  
  /* gets the mappings */
  ac_asm_map_list *ml = ac_asm_get_mapping_list();
  
  /* for each conversion specifier... */
  while (ml != NULL) {

    if (!(ml->used_where & 1)) {  /* skip if not an operand marker */
      ml = ml->next;
      continue;
    }

    ac_asm_symbol *s = ml->symbol_list;

    /* ... and for each symbol, write the mapping */
    while (s != NULL) {
      fprintf(output, "%s{\"%s\",\t\"%s\",\t%d},\n", IND1, s->symbol, ml->marker, 
            s->value);

      s = s->next;
    }

    ml = ml->next;
  }

  fclose(output);
  return 1; 
}


/*
  Pseudo instruction table generation

  TODO: describe the structure

*/
int CreatePseudoOpsTable(const char *optable_filename) 
{
  FILE *output;

  if ((output = fopen(optable_filename, "w")) == NULL) 
    return 0;

  /* gets the list from the parser */
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();

  /* first entry is always NULL */
  fprintf(output, "%sNULL", IND1);

  /* for each insn... */
  while (asml != NULL) {

    /* ... if it's a pseudo-instr, write the list of instructions in sequence */
    if (asml->pseudolist != NULL) {

      strlist *pl = asml->pseudolist;
      while (pl != NULL) {
        fprintf(output, ",\n%s\"%s\"", IND1, pl->str);
        pl = pl->next;
      }
    
      fprintf(output, ",\n%sNULL", IND1);
    }

    asml = asml->next;
  }

  fclose(output);
  return 1; 
}




/*
 * module function implementations 
 */

static void create_operand_string(ac_asm_insn *insn, char **output) 
{
  char *s = *output;
  char *lit = insn->op_literal;

  *s = '\0';
  
  ac_operand_list *opP = insn->operands;  

  while (*lit != '\0') {

    if (*lit == '%') {
      *s = *lit;
      s++;
      lit++;

      char *ls = opP->str;
      while (*ls != '\0') {
        *s = *ls;
        s++;
        ls++;          
      }

      *s = ':';

      ac_asm_insn_field *fP = opP->fields;
      while (fP != NULL) {
         
        s++;
        sprintf(s, "%d", fP->id);
        while (*s >= '0' && *s <= '9') s++;

        *s = ':'; 

        s++;
        sprintf(s, "%d", fP->reloc_id);
        while (*s >= '0' && *s <= '9') s++;
        
        fP = fP->next;
        if (fP != NULL) 
          *s = '+';
      }

      *s = ':';
      s++;

      opP = opP->next;
      /* TODO: check for NULL pointer */
       
    }     
    else if (*lit == '\\') {
       *s = *lit;
       s++;
       lit++;
       if (*lit == '\\') {
         *s = *lit;
         s++;
         lit++;
       }
    }

    *s = *lit;
    s++;
    lit++;
  }
  *s = '\0';  
}

static unsigned int encode_insn_field(unsigned int field_value, unsigned insn_size, unsigned fbit, unsigned fsize) {

  // TODO: see if the 'val' field can accept constant values (this is not being checked atm)
  // TODO: check if it is correct for different word size instructions
  unsigned int mask1 = 0xffffffff;
  unsigned int mask2 = 0xffffffff;
  unsigned int return_value;

  mask1 <<= (insn_size-(fbit+1));
  mask2 >>= fbit+1-fsize;
  return_value = ((field_value << (insn_size-(fbit+1)) & (mask1 & mask2)));

  return return_value;
}
