/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      opcodes.c
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
 * @brief     Opcodes library related code (implementation)
 * 
 * @attention Copyright (C) 2002-2008 --- The ArchC Team
 *
 */


#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "opcodes.h"

/* This is almost the same structure in /include/opcode/xxxxx.h (acasm_operand)
 TODO: find a way to unify the structures in a single file
 */
typedef struct _oper_list {
  unsigned int id;   /* operand id */
  char *name;  /* operand type name */
  operand_type type;
  int mod_type;
  int mod_addend;
  int is_list;
  unsigned int fields;
  unsigned int format_id;
  unsigned int reloc_id;
  unsigned int fields_positions;
  struct _oper_list *next;
} oper_list;

static oper_list *operand_list = NULL;

typedef struct _mod_list {
  int id;
  char *name;
  struct _mod_list *next;
} mod_list;

static mod_list *modifier_list = NULL;

/*
 * module function prototypes
 */
static oper_list *find_operand(oper_list *opl);
static void create_operand_string(ac_asm_insn *insn, char **output);
static unsigned int encode_insn_field(unsigned int field_value, unsigned insn_size, unsigned fbit, unsigned fsize);
static unsigned int encode_dmask_field(unsigned insn_size, unsigned fbit, unsigned fsize);
static int get_compatible_oper_id(operand_type opt);
static void create_modifier_list();

void create_operand_list()
{
  create_modifier_list();
  
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();

  /* for every instruction */
  while (asml != NULL) {
    if (asml->insn == NULL) { /* check only native insn */
      asml = asml->next;
      continue;
    }

    ac_operand_list *opP = asml->operands;

    /* for every operand */
    while (opP != NULL) {
 
      oper_list *oper = (oper_list *) malloc (sizeof(oper_list));

      oper->name = (char *) malloc(strlen(opP->str)+1);

      strcpy(oper->name, opP->str);

      oper->type = opP->type;
    
      oper->mod_type = opP->modifier.type;
      oper->mod_addend = opP->modifier.addend;

      oper->is_list = opP->is_list;
      
      oper->fields = encode_fields(opP->fields);
      oper->fields_positions = encode_fields_positions(opP->fields, get_insn_size(asml));

      oper->reloc_id = 0; /*opP->reloc_id;*/
      oper->format_id = get_format_id(asml->insn->format);
   
      oper->next = NULL;

      oper_list *opfound = find_operand(oper);

      if (opfound == NULL) { /* operand not defined yet */
        if (operand_list == NULL) {
          oper->id = 0;
          opP->oper_id = 0;
          operand_list = oper;
        }
        else {
          oper_list *otmp = operand_list;
          while (otmp->next != NULL) 
            otmp = otmp->next;

          oper->id = otmp->id+1; 
          opP->oper_id = oper->id;
          otmp->next = oper;
        }
      }
      else {
        opP->oper_id = opfound->id;
        
        free(oper->name);
        free(oper);
      }
      
      opP = opP->next;
    }

    asml = asml->next;
  }

}

void update_oper_list(int oper_id, unsigned int reloc_id)
{

  oper_list *otmp = operand_list;

  while (otmp != NULL) {
    if (otmp->id == oper_id) {
      otmp->reloc_id = reloc_id;
      return;
    }
    otmp = otmp->next;
  }

  internal_error();
}


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
    unsigned long dmask = 0x00;
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
        /* for each decoding field, finds its place in the format and encode it in 
        the base_image variable */
          {
            ac_dec_list *pdl = asml->insn->dec_list;

            while (pdl != NULL) {       
              ac_dec_field *pdf = pfrm->fields;
              while ((pdf != NULL) && strcmp(pdl->name, pdf->name)) pdf = pdf->next;
         
              if (pdf == NULL) internal_error();
              else {
                base_image |= encode_insn_field(pdl->value, pfrm->size, pdf->first_bit, pdf->size);
                dmask |= encode_dmask_field(pfrm->size, pdf->first_bit, pdf->size);
              }
                 
              pdl = pdl->next;         
            }  

            ac_const_field_list *cfP = asml->const_fields;
            while (cfP != NULL) {
              base_image |= encode_insn_field(cfP->value, pfrm->size, cfP->field.first_bit, cfP->field.size);
              dmask |= encode_dmask_field(pfrm->size, cfP->field.first_bit, cfP->field.size);                
              cfP = cfP->next;
            }
          }

      }      
    }
    fprintf(output, "0x%08lX,\t", base_image);      
    /*----------------------------------------------------------------*/
  
    /* format id */
    /*----------------------------------------------------------------*/
    fprintf(output, "%lu,\t", format_id);
    /*----------------------------------------------------------------*/

    /* if a pseudo, saves the pseudo-instr table instruction index */
    /*----------------------------------------------------------------*/
    if (asml->pseudolist != NULL) {
      fprintf(output, "%lu", pseudo_idx);
      pseudo_idx += asml->num_pseudo+1;
    }
    else
      fprintf(output, "0");
    /*----------------------------------------------------------------*/

    /***
     Optional statistic field
     ***/
    fprintf(output, ",\t0");

    /* Mask, used for disassemble */
    fprintf(output, ", \t0x%08lX", dmask);

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
  ac_asm_symbol *s = ac_asm_get_mapping_list();
  

  /* ... and for each symbol, write the mapping */
  while (s != NULL) {
    fprintf(output, "%s{\"%s\",\t\"%s\",\t%d},\n", IND1, s->symbol, s->map_marker, 
          s->value);

    s = s->next;
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


int CreateOperandTable(const char *optable_filename) 
{
  FILE *output;

  if ((output = fopen(optable_filename, "w")) == NULL) 
    return 0;

  oper_list *opL = operand_list;

  while (opL != NULL) {

    fprintf(output, "%s{\"%s\",\t // (%d)\n\t", IND1, opL->name, opL->id);
    fprintf(output, "%d,\t", opL->type);
    fprintf(output, "%d,\t", opL->mod_type);
    fprintf(output, "%d,\t", opL->is_list);
    fprintf(output, "%d,\t", opL->mod_addend);
    fprintf(output, "%u,\t", opL->fields);
    fprintf(output, "%u,\t", opL->format_id);
    fprintf(output, "%u,\t", opL->reloc_id + 36);
    fprintf(output, "0x%08x ", opL->fields_positions);

    fprintf(output, "},\n"); 

    opL = opL->next;
  }

  fclose(output);
  return 1; 
}

int CreateModifierEnum(const char *filename)
{
  char *strP;
  FILE *output;

  if ((output = fopen(filename, "w")) == NULL) 
    return 0;

  mod_list *modifier = modifier_list;

  fprintf(output, "typedef enum {mod_%s = %d", modifier->name, modifier->id);
  modifier = modifier->next;
  while (modifier != NULL) {
    
    fprintf(output, ", mod_%s = %d", modifier->name, modifier->id);

    modifier = modifier->next;
  }
  fprintf(output, "} operand_modifier;\n");

  fclose(output);
  return 1; 
}

int CreateModifierProt(const char *filename)
{
  char *strP;
  FILE *output;

  if ((output = fopen(filename, "w")) == NULL) 
    return 0;

  mod_list *modifier = modifier_list;

  while (modifier != NULL) {
    
    fprintf(output, "extern void modifier_%s_encode(mod_parms *reloc);\n", modifier->name);
    fprintf(output, "extern void modifier_%s_decode(mod_parms *reloc);\n", modifier->name);

    modifier = modifier->next;
  }

  fclose(output);
  return 1; 
}

int CreateFormatStruct(const char *filename)
{
  char *strP;
  FILE *output;

  if ((output = fopen(filename, "w")) == NULL) 
    return 0;

  ac_dec_format *pfrm = format_ins_list;

  /* for every format */
  while (pfrm != NULL) {
    ac_dec_field *fields = pfrm->fields;
    fprintf(output, "%sstruct {\n", IND2);

    while (fields != NULL) {
      fprintf(output, "%s unsigned int %s;\n", IND3, fields->name);
      fields = fields->next;
    }
 
    fprintf(output, "%s} %s;\n", IND2, pfrm->name);

    pfrm = pfrm->next;
  }


  fclose(output);
  return 1; 
}

/* which: 0 = encode modifiers, 1 = decode modifiers */
int CreateModifierPtr(const char *filename, int which)
{
  char *strP;
  FILE *output;

  if ((output = fopen(filename, "w")) == NULL) 
    return 0;

  mod_list *modifier = modifier_list;

  while (modifier != NULL) {
    if (which == 0) 
      fprintf(output, "&modifier_%s_encode,\n", modifier->name);
    else
      fprintf(output, "&modifier_%s_decode,\n", modifier->name); 

    modifier = modifier->next;
  }

  fclose(output);
  return 1; 
}



/*
 * module function implementations 
 *
 */

/*
 *
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

      if (opP->oper_id == -1) { /* pseudo operand */
        opP->oper_id = get_compatible_oper_id(opP->type);
      } 
 
      sprintf(s, "%d", opP->oper_id);
      while (*s >= '0' && *s <= '9') s++;

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
             
        if (*lit == '%') {
          *s = *lit;
          s++;
          lit++;
        }
      }
    }
    else {
      *s = *lit;
      s++;
      lit++;
    }
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

static unsigned int encode_dmask_field(unsigned insn_size, unsigned fbit, unsigned fsize) 
{
  // TODO: see if the 'val' field can accept constant values (this is not being checked atm)
  unsigned int mask1 = 0xffffffff;
  unsigned int mask2 = 0xffffffff;
  unsigned int field_value=0;
  
  int i=0;
  for (i;i<fsize;i++){
    field_value = (field_value<<1) + 1;
  }

  mask1 <<= (insn_size-(fbit+1));
  mask2 >>= fbit+1-fsize;
  return ((field_value << (insn_size-(fbit+1)) & (mask1 & mask2)));
}


/* -1 -> not found  */
static oper_list *find_operand(oper_list *opl)
{
  oper_list *otmp = operand_list;

  while (otmp != NULL) {
    if (!strcmp(otmp->name, opl->name) &&
         otmp->type == opl->type &&
	 otmp->is_list == opl->is_list &&
         otmp->mod_type == opl->mod_type &&
         otmp->mod_addend == opl->mod_addend &&
         otmp->fields == opl->fields &&
         otmp->format_id == opl->format_id)
      return otmp;

    otmp = otmp->next;
  }

  return NULL;
}

/* returns -1 in case no one is found */
static int get_compatible_oper_id(operand_type opt)
{
  oper_list *opP = operand_list;
  int retvalue = -1;
  
  while (opP != NULL) {
    if (opP->type == opt) 
      return (int) opP->id;

    opP = opP->next;
  }

  return retvalue;
}


static void create_modifier_list()
{
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();
  mod_list *modp = NULL;

  /*
   First item -> default
  */
  mod_list *modifier = (mod_list *) malloc(sizeof(mod_list));
  modifier->name = (char *) malloc (strlen("default")+1);
  strcpy(modifier->name, "default");
  modifier->id = 0;
  modifier->next = NULL;

  modifier_list = modifier;


  /* for every instruction */
  while (asml != NULL) {
//    if (asml->insn == NULL) { /* check only native insn */
//      asml = asml->next;
//      continue;
//    }

    ac_operand_list *opP = asml->operands;

    /* for every operand */
    while (opP != NULL) {

      if (opP->modifier.name == NULL) {
        opP->modifier.type = 0;
        opP = opP->next;
        continue;
      }

      /* try finding an already-created modifier with the same name */
      modifier = modifier_list;
      while (modifier != NULL) {
        if (!strcmp(modifier->name, opP->modifier.name)) break;
     
        modifier = modifier->next;
      }

      /* not found, create a new modifier item and insert in the list */
      if (modifier == NULL) {
        int last_id = 0;
        modifier = (mod_list *) malloc(sizeof(mod_list));
        modifier->name = (char *) malloc (strlen(opP->modifier.name)+1);
        strcpy(modifier->name, opP->modifier.name);

        /* insert at the end of the list */
        /* NOTE: modifier_list MUST NOT be NULL */
        for (modp=modifier_list; modp->next != NULL; modp = modp->next)
          last_id++;

        modifier->id = last_id+1;
        modifier->next = NULL;
        modp->next = modifier;
      }
 
      opP->modifier.type = modifier->id; 
      opP = opP->next;
    }

    asml = asml->next;
  }
}

