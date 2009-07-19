/* ex: set tabstop=2 expandtab:
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/

/**
 * @file      ac_decoder.c
 * @author    Sandro Rigo
 *            Marcus Bartholomeu
 *            Marcelo de Almeida Oliveira (contributor)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Wed, 07 Jun 2006 16:49:59 -0300
 *
 * @brief     Creates the decoder for the target ISA.
 *            This file contains the functions needed to create the 
 *            decoder for the target ISA described in the ArchC file.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ac_decoder.h"


//! External function that gets the required bits from the buffer
unsigned long long GetBits(void *buffer, int *quant, int last, int quantity, int sign);




/*! This function copies a string into a new allocated memory region
  \param s The string to be copied
  \return A new copy of the string into a new allocated memory region
 */
char *NewString(const char *s)
{
  char *returnValue = (char *) malloc(strlen(s) + 1);
  strcpy(returnValue, s);

  return returnValue;
}


void ShowDecField(ac_dec_field *f)
{
  while (f) {
    printf("\tac_dec_field\n\t  Name : %s\n\t  Size : %d\n\t  First: %d\n\t  ID   : %d\n\t  Value: %ld\n",
           f -> name, f -> size, f -> first_bit, f -> id, f -> val);
    f = f -> next;
  }
}


void ShowDecFormat(ac_dec_format *f)
{
  while (f) {
    printf("ac_dec_format\nName: %s\nFields:\n", f -> name);
    ShowDecField(f -> fields);
    f = f -> next;
  }
}


void ShowDecodeList(ac_dec_list *l)
{
  while (l) {
    printf("\tac_dec_list\n\t  Name : %s\n\t  Value: %d\n", l -> name, l -> value);
    l = l -> next;
  }
}


void ShowDecInstr(ac_dec_instr *i)
{
  while (i) {
    printf("ac_dec_instr\nName    : %s\nMnemonic: %s\nASM     : %s\nFormat  : %s\nDecode List:\n",
           i -> name, i -> mnemonic, i -> asm_str, i -> format);
    ShowDecodeList(i -> dec_list);
    i = i -> next;
  }
}


void ShowDecoder(ac_decoder *d, unsigned level)
{
  char *ident = (char *) malloc(level * 4 + 1);
  unsigned counter;

  for (counter = 0; counter < level * 4; counter ++)
    ident[counter] = ' ';

  ident[level * 4] = '\0';

  while (d) {
    printf("%sac_decoder:\n", ident);
    printf("%s Field:\n", ident);
    printf("%s  Name: %s\n%s  Val : %d\n", ident, d -> check -> name, ident, d -> check -> value);
    if (d -> found) {
      printf("%s Instruction Found:\n", ident);
      printf("%s  Name    : %s\n%s  Mnemonic: %s\n", ident, d -> found -> name, ident, d -> found -> mnemonic);
    }
    else
      ShowDecoder(d -> subcheck, level + 1);

    d = d -> next;
  }

  free(ident);
}


void MemoryError(char *fileName, long lineNumber, char *functionName)
{
  fprintf(stderr, "%s: %ld: Not enough memory at %s", fileName, lineNumber, functionName);
  exit(1);
}


//! Compare two fields. Used in qsort and bsearch
int CompareFields(const void *p1, const void *p2)
{
  const ac_dec_field *f1 = (const ac_dec_field *) p1, *f2 = (const ac_dec_field *) p2;

  return strcmp(f1 -> name, f2 -> name);
}


/* Fields which have the same name, should have the same position inside the instruction
   \return 0 if fields have the same name but different position, -1 if fields are equal, 1 otherwise
*/

int CheckFields(const ac_dec_field *f1, const ac_dec_field *f2)
{
  if (!strcmp(f1 -> name, f2 -> name)) {
    if (strcmp(f1 -> name, "0")) {
      if ((f1 -> size != f2 -> size) || (f1 -> first_bit != f2 -> first_bit)) {
        fprintf(stderr, "Field '%s' defined in two formats but with different size or position.\n", f1 -> name);
        return 0;
      }
    }
    else
      return 1;

    return -1;
  }
  return 1;
}


/* ac_dec_field *CopyDecField(ac_dec_field *dst, const ac_dec_field *src) */
/* { */
/*   *dst = *src; */
/*   dst -> name = NewString(src -> name); */
/* } */


/* ac_dec_field *NewDecField(ac_dec_field *p) */
/* { */
/*   ac_dec_field *f = (ac_dec_field *) malloc(sizeof(ac_dec_field)); */

/*   CopyDecField(f, p); */
/*   f -> next = NULL; */
/* } */


void FreeDecField(ac_dec_field *f)
{
  free(f -> name);
  free(f);
}


ac_dec_field *FindDecField(ac_dec_field *fields, char *name)
{
  while (fields) {
    if (!strcmp(fields -> name, name))
      return fields;

    fields = fields -> next;
  }

  return NULL;
}

void PutCommonFieldsIds(ac_dec_field *common_fields, ac_dec_format *formats)
{
  /* Our reference fields are in the first format. These fields should have IDs
     given by PutIDs. Now we check theses IDs and copy them to the common fields.
     If a field is common to all formats, then it obviously must be in the first
     format as well, so it suffices to have only the first format as reference.
  */
  
  ac_dec_field *p = common_fields;

  /* Iterate through all common fields, and check its ID in the reference fields list */
  while (p) {
    ac_dec_field *ref = formats->fields;
    while (ref) {    
      if (CheckFields(ref, p) == -1) {
        p->id = ref->id;
        break;
      }
      ref = ref->next;
    }
    p = p->next;
  }
}


ac_dec_field *PutIDs(ac_dec_format *formats, unsigned nFields)
     /* TODO:
        - porque o indice e o id n� s� iguais? (poderia pegar o nome pelo id) (id 0 reservado)
        - porque n� muda o id na mesma estrutura recebida?
        - nFields �na verdade o limit!!! (== id de maior valor) 
      */
{
  ac_dec_field *fields, *tmp;
  ac_dec_format *f = formats;
  unsigned limit = 0, index, result;
  char found;
  int error = 0;

  // Allocates memory
  fields = (ac_dec_field *) malloc(nFields * sizeof(ac_dec_field));

  // Creates a vector with unique occurrences of all fields
  while (f) {
    tmp = f -> fields;
    while (tmp) {
      found = 0;
      for (index = 0; index < limit; index ++) {
        result = CheckFields(&(fields[index]), tmp);
        if (result == -1) {
          found = 1;
          tmp -> id = fields[index].id;
          break;
        }
        else if (result == 0) error++;
      }

      if (!found) {
        tmp -> id = limit + 1;
        fields[limit] = *tmp;
        fields[limit].name = NewString(tmp -> name);

        if (limit != 0)
          fields[limit - 1].next = &(fields[limit]);

        fields[limit++].next = NULL;
      }

      tmp = tmp -> next;
    }
    f = f -> next;
  }

  if (error) exit(1);

  return fields;
}


ac_decoder *AddToDecoder(ac_decoder *decoder, ac_dec_instr *instruction, ac_dec_field *fields)
{
  ac_decoder *d = decoder, *base = decoder;
  //ac_dec_field *f;
  ac_dec_list *l = instruction -> dec_list;
  //enum {IncludeSubCheck, IncludeNext} action;

  if (l == NULL) {
    fprintf(stderr, "Error: Instruction %s doesn't have a decode list.\n", instruction -> name);
    exit(1);
  }

  if (d == NULL) {
    base = d = (ac_decoder *) malloc(sizeof(ac_decoder));
    d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
    d -> check -> name = NewString(l -> name);
    d -> check -> value = l -> value;
    d -> found = NULL;
    d -> subcheck = NULL;
    d -> next = NULL;
  }

  while (l) {
    // Same field and same value to check
    if ((!strcmp(d -> check -> name, l -> name)) && (d -> check -> value == l -> value))
      if (l -> next == NULL) {            // This is the last check
        d -> found = instruction;
        l = l -> next;
      }
      else {
        l = l -> next;
        if (d -> subcheck != NULL)        // Include in subcheck
          d = d -> subcheck;
        else {
          d -> subcheck = (ac_decoder *) malloc(sizeof(ac_decoder));
          d = d -> subcheck;
          d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
          d -> check -> name = NewString(l -> name);
          d -> check -> value = l -> value;
          d -> found = NULL;
          d -> subcheck = NULL;
          d -> next = NULL;
        }
      }
    else {
      if (d -> next != NULL)
        d = d -> next;
      else {
        d -> next = (ac_decoder *) malloc(sizeof(ac_decoder));
        d = d -> next;
        d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
        d -> check -> name = NewString(l -> name);
        d -> check -> value = l -> value;
        d -> found = NULL;
        d -> subcheck = NULL;
        d -> next = NULL;
      }
    }
  }

  d -> found = instruction;
  d -> subcheck = NULL;

  return base;
}


ac_decoder_full *CreateDecoder(ac_dec_format *formats, ac_dec_instr *instructions)
{
  ac_dec_field *field, *allFields;
  ac_dec_format *format = formats;
  //ac_dec_list *list;
  ac_dec_instr *instr = instructions;
  ac_decoder *dec;
  ac_decoder_full *full;
  unsigned nFields = 0, nFormats = 0;

  while (format) {
/*     int format_size = 0; */
    nFormats ++;
    field = format -> fields;
    while (field) {
/*       format_size += field->size; */
      nFields++;
      field = field -> next;
    }
/*     format->size = format_size; */
    format = format -> next;
  }
  nFields++;
  allFields = PutIDs(formats, nFields);

  dec = NULL;

  while (instr) {
    dec = AddToDecoder(dec, instr, allFields);
    instr->size = FindFormat(formats, instr->format)->size / 8;  //bits to bytes
    instr = instr -> next;
  }

  full = malloc(sizeof(ac_decoder_full));

  full -> decoder = dec;
  full -> formats = formats;
  full -> fields = allFields;
  full -> instructions = instructions;
  full -> nFields = nFields;

  return full;
}


ac_dec_instr *FindInstruction(ac_decoder_full *decoder, unsigned char *buffer, int quant)
{
  ac_decoder *d = decoder -> decoder;
  //ac_dec_list *check;
  ac_dec_field *field = 0;
  long long field_value;
  //char byte;

  ac_decoder *chosenPath[64]; // usar uma constante = MAX_DECODER_DEPTH
  int chosenPathPos = 0;
  chosenPath[chosenPathPos] = d;

  while (d) {
    if (!field) {
      field = FindDecField(decoder -> fields, d -> check -> name);
      field_value = GetBits(buffer, &quant, field -> first_bit, field -> size, field -> sign);
    }

    //fprintf(stderr, "Decoder - FieldName: %s ValueRequired: %d ValueFound: %d\n",
    //        d -> check -> name, d -> check -> value, field_value);

    if (field_value == d -> check -> value) {
      if (d -> found) {
        //fprintf(stderr, "Instruction %s has been found.\n", d -> found -> name);
        return d -> found;
      } else {
        //fprintf(stderr, "Following d -> subcheck branch in the decoder tree. \n");
        chosenPath[++chosenPathPos] = d -> subcheck;
        d = d -> subcheck;
        field = 0;
      }
    } else {
      //fprintf(stderr, "Following d->next branch in the decoder tree. \n");
      chosenPath[chosenPathPos] = d -> next;
      d = d -> next;
      if (d && strcmp(d->check->name, field->name) != 0)
        field=0;
    }

    while ((d == NULL) && (chosenPathPos > 0)) {
      //fprintf(stderr, "Backtracking in the decoder tree.\n");
      d = chosenPath[--chosenPathPos];
      chosenPath[chosenPathPos] = d -> next;
      d = d -> next;
      if (d && strcmp(d -> check -> name, field -> name) != 0)
        field = 0;
    }
  }

  return NULL;
}


ac_dec_format *FindFormat(ac_dec_format *formats, char *name)
{
  ac_dec_format *format = formats;

  while(format && (strcmp(format -> name, name) != 0)) {
    format = format -> next;
  }
  if (!format) {
    fprintf(stderr, "Invalid format name %s.\n", name);
    exit(1);
  }
  return format;
}


ac_dec_instr *GetInstrByID(ac_dec_instr *instr, int id)
{
  for (; instr && instr->id != id;) {
    instr = instr->next;
  }
  if (!instr) {
    fprintf(stderr, "Invalid instruction ID %d.\n", id);
    exit(1);
  }
  return instr;
}


unsigned *DecodeAsInstruction(ac_decoder_full * decoder, ac_dec_instr *instruction, unsigned char *buffer, int quant)
{
  static unsigned *fields = 0;
  unsigned counter;
  ac_dec_field *field;
  //ac_dec_list *list = instruction -> dec_list;
  ac_dec_field *list ;

  ac_dec_format *format;

  //!Allocate only the first time
  if (!fields) fields = (unsigned *) malloc(sizeof(unsigned) * decoder -> nFields);

  //!Looking for the instruction format structure.
  format = FindFormat(decoder->formats, instruction->format);

  //!Get Format's field list
  list = format -> fields;

  for (counter = 1; counter < decoder -> nFields; counter ++)
    fields[counter] = 0;

  while (list) {
    field = FindDecField(decoder -> fields, list -> name);
    if (!field) {
      fprintf(stderr, "Invalid field name %s.\n", list -> name);
      exit(1);
    }
    fields[field -> id] = GetBits(buffer, &quant, field -> first_bit, field -> size, field -> sign);
    list = list -> next;
  }

  fields[0] = instruction -> id;
  return fields;
}


unsigned *Decode(ac_decoder_full *decoder, unsigned char *buffer, int quant)
{
  ac_dec_instr *instruction;

  instruction = FindInstruction(decoder, buffer, quant);

  if (instruction != NULL) {
    return DecodeAsInstruction(decoder, instruction, buffer, quant);
  }

  return NULL;
}
