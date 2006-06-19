/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_decoder_rt.cpp
 * @author    Marcus Bartholomeu
 *            Marcelo de Almeida Oliveira (Contributor)
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   version?
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
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
#include "ac_decoder_rt.H"

using std::cerr;

ostream& operator << (ostream& os, ac_dec_field& adf) {
  ac_dec_field* f = &adf;
  while (f) {
    os << "\tac_dec_field\n\t  Name : " << f->name
       << "\n\t  Size : " << f->size
       << "\n\t  First: " << f->first_bit
       << "\n\t  ID   : " << f->id
       << "\n\t  Value: " << f->val << endl;
    f = f->next;
  }
  return os;
}

ostream& operator << (ostream& os, ac_dec_format& adf) {
  ac_dec_format* f = &adf;
  while (f) {
    os << "ac_dec_format\n  Name: " << f->name
       << "\n  Fields: " << f->fields << endl;
    f = f->next;
  }
  return os;
}

ostream& operator << (ostream& os, ac_dec_list& adl) {
  ac_dec_list* l = &adl;
  while (l) {
    os << "\tac_dec_list\n\t  Name : " << l->name
       << "\n\t  Value: " << l->value << endl;
    l = l -> next;
  }
  return os;
}

ostream& operator << (ostream& os, ac_dec_instr& adi) {
  ac_dec_instr* i = &adi;
  while (i) {
    os << "ac_dec_instr"
       << "\nName    : " << i->name
       << "\nMnemonic: " << i->mnemonic
       << "\nASM     : " << i->asm_str
       << "\nFormat  : " << i->format
       << "\nDecode List: " << i->dec_list << endl;
    i = i -> next;
  }
  return os;
}

void ac_decoder::ShowDecoder(unsigned level) {
  ac_decoder* d = this;
  char* ident = new char[level * 4 + 1];
  unsigned counter;

  for (counter = 0; counter < level * 4; counter ++)
    ident[counter] = ' ';

  ident[level * 4] = '\0';

  while (d) {
    printf("%sac_decoder:\n", ident);
    printf("%s Field:\n", ident);
    printf("%s  Name: %s\n%s  Val : %d\n", ident, d -> check -> name.c_str(), ident, d -> check -> value);
    if (d -> found) {
      printf("%s Instruction Found:\n", ident);
      printf("%s  Name    : %s\n%s  Mnemonic: %s\n", ident, d -> found -> name.c_str(), ident, d -> found -> mnemonic.c_str());
    }
    else
      (d->subcheck)->ShowDecoder(level + 1);

    d = d -> next;
  }

  delete[] ident;
}

// USEFUL I guess
void MemoryError(char *fileName, long lineNumber, char *functionName)
{
  fprintf(stderr, "%s: %ld: Not enough memory at %s", fileName, lineNumber, functionName);
  exit(1);
}

/* Fields which have the same name, should have the same position inside the instruction
   \return 0 if fields have the same name but different position, -1 if fields are equal, 1 otherwise
*/
// USEFUL?
int ac_dec_field::CheckFields(const ac_dec_field& f2) const {
  if (name == f2.name) {
    if (name != "0") {  
      if ((size != f2.size) || (first_bit != f2.first_bit)) {
	cerr << "Field '" << name
	     << "' defined in two formats but with different size or position."
	     << endl;
        return 0;
      }
    }
    else
      return 1;
    
    return -1;
  }
  return 1;
}

// Turns into method
ac_dec_field* ac_dec_field::FindDecField(string name) {
  ac_dec_field* fields = this;
  while (fields) {
    if (fields->name == name)
      return fields;
    
    fields = fields->next;
  }

  return 0;
}

// USEFUL?
ac_dec_field* ac_dec_field::PutIDs(ac_dec_format *formats, unsigned nFields)
     /* TODO:
        - porque o indice e o id n� s� iguais? (poderia pegar o nome pelo id) (id 0 reservado)
        - porque n� muda o id na mesma estrutura recebida?
        - nFields �na verdade o limit!!! (== id de maior valor) (ao alocar, nFields+1, pois tem o 0)
      */
{
  ac_dec_field* fields;
  ac_dec_field* tmp;
  ac_dec_format* f = formats;
  unsigned limit = 0, index;
  int result;
  char found;
  int error = 0;

  // Allocates memory
//   fields = (ac_dec_field *) malloc(nFields * sizeof(ac_dec_field));
  fields = new ac_dec_field[nFields];

  // Creates a vector with unique occurrences of all fields
  while (f) {
    tmp = f -> fields;
    while (tmp) {
      found = 0;
      for (index = 0; index < limit; index ++) {
        result = fields[index].CheckFields(*tmp);
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
        fields[limit].name = tmp -> name;
        
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

// ac_decoder method?
ac_decoder* ac_decoder::AddToDecoder(ac_decoder* decoder, ac_dec_instr* instruction, ac_dec_field* fields)
{
  ac_decoder *d = decoder, *base = decoder;
  //ac_dec_field *f;
  ac_dec_list *l = instruction -> dec_list;
  //enum {IncludeSubCheck, IncludeNext} action;
  
  if (l == NULL) {
    fprintf(stderr, "Error: Instruction %s doesn't have a decode list.\n", instruction -> name.c_str());
    exit(1);
  }

  if (d == NULL) {
//     base = d = (ac_decoder *) malloc(sizeof(ac_decoder));
//     d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
//     d -> check -> name = NewString(l -> name);
    base = d = new ac_decoder();
    d -> check = new ac_dec_list();
    d -> check -> name = l -> name;
    d -> check -> value = l -> value;
    d -> found = NULL;
    d -> subcheck = NULL;
    d -> next = NULL;
  }
    
  while (l) {
    // Same field and same value to check
    if ((d -> check -> name == l -> name) && (d -> check -> value == l -> value))
      if (l -> next == NULL) {            // This is the last check
        d -> found = instruction;
        l = l -> next;
      }
      else {
        l = l -> next;
        if (d -> subcheck != NULL)        // Include in subcheck
          d = d -> subcheck;
        else {
//           d -> subcheck = (ac_decoder *) malloc(sizeof(ac_decoder));
          d -> subcheck = new ac_decoder();
          d = d -> subcheck;
//           d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
//           d -> check -> name = NewString(l -> name);
          d -> check = new ac_dec_list();
          d -> check -> name = l -> name;
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
//         d -> next = (ac_decoder *) malloc(sizeof(ac_decoder));
        d -> next = new ac_decoder();
        d = d -> next;
//         d -> check = (ac_dec_list *) malloc(sizeof(ac_dec_list));
//         d -> check -> name = NewString(l -> name);
        d -> check = new ac_dec_list();
        d -> check -> name = l -> name;
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

// ac_decoder_full static method, or constructor? :-D
ac_decoder_full *ac_decoder_full::CreateDecoder(ac_dec_format *formats, ac_dec_instr *instructions, ac_dec_prog_source* source)
{
  ac_dec_field *field, *allFields;
  ac_dec_format *format = formats;
  //ac_dec_list *list;
  ac_dec_instr *instr = instructions;
  ac_decoder *dec = 0;
  ac_decoder_full *full = 0;
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
  allFields = ac_dec_field::PutIDs(formats, nFields);

  // dec = new ac_decoder();

  while (instr) {
    dec = ac_decoder::AddToDecoder(dec, instr, allFields);
    instr->size = ac_dec_format::FindFormat(formats, instr->format.c_str())->size / 8;  //bits to bytes
    instr = instr -> next;
  }

//   full = malloc(sizeof(ac_decoder_full));
  full = new ac_decoder_full();

  full -> decoder = dec;
  full -> formats = formats;
  full -> fields = allFields;
  full -> instructions = instructions;
  full -> nFields = nFields;
  full -> prog_source = source;
  
  return full;
}

// ac_dec_instr method?
ac_dec_instr* ac_dec_instr::FindInstruction(ac_decoder_full *decoder, unsigned char *buffer, int quant)
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
      field = decoder->fields->FindDecField(d -> check -> name);
      field_value = decoder->prog_source->GetBits(buffer, &quant, field -> first_bit, field -> size, field -> sign);
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
      if (d && (d->check->name != field->name))
        field=0;
    }

    while ((d == NULL) && (chosenPathPos > 0)) {
      //fprintf(stderr, "Backtracking in the decoder tree.\n");
      d = chosenPath[--chosenPathPos];
      chosenPath[chosenPathPos] = d -> next;
      d = d -> next;
      if (d && (d -> check -> name != field -> name))
        field = 0;
    }
  }

  return NULL;
}

// ac_dec_format method?
ac_dec_format* ac_dec_format::FindFormat(ac_dec_format *formats, const char *name)
{
  ac_dec_format *format = formats;

  while(format && (strcmp(format -> name.c_str(), name) != 0)) {
    format = format -> next;
  }
  if (!format) {
    fprintf(stderr, "Invalid format name %s.\n", name);
    exit(1);
  }
  return format;
}

// ac_dec_instr method
ac_dec_instr* ac_dec_instr::GetInstrByID(ac_dec_instr *instr, unsigned id)
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

// ac_decoder_full method?
unsigned* ac_decoder_full::DecodeAsInstruction(ac_decoder_full * decoder, ac_dec_instr *instruction, unsigned char *buffer, int quant)
{
  static unsigned* fields = 0;
  unsigned counter;
  ac_dec_field *field;
  //ac_dec_list *list = instruction -> dec_list;
  ac_dec_field *list ;

  ac_dec_format *format;

  //!Allocate only the first time
//   if (!fields) fields = (unsigned *) malloc(sizeof(unsigned) * decoder -> nFields);
  if (!fields) {
    fields = new unsigned[decoder -> nFields];
  }

  //!Looking for the instruction format structure.
  format = ac_dec_format::FindFormat(decoder->formats, instruction->format.c_str());
  
  //!Get Format's field list
  list = format -> fields;

  for (counter = 1; counter < decoder -> nFields; counter ++)
    fields[counter] = 0;

  while (list) {
//     field = FindDecField(decoder -> fields, list -> name.c_str());
    field = decoder->fields->FindDecField(list->name);
    if (!field) {
      fprintf(stderr, "Invalid field name %s.\n", list -> name.c_str());
      exit(1);
    }
    fields[field -> id] = decoder->prog_source->GetBits(buffer, &quant, field -> first_bit, field -> size, field -> sign);
    list = list -> next;
  }

  fields[0] = instruction -> id;
  return (unsigned*) fields;
}

// ac_decoder_full method?
unsigned* ac_decoder_full::Decode(unsigned char *buffer, int quant)
{
  ac_dec_instr *instruction;
  
  instruction = ac_dec_instr::FindInstruction(this, buffer, quant);
  
  if (instruction != NULL) {
    return DecodeAsInstruction(this, instruction, buffer, quant);
  }

  return NULL;
}
