/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/*  Generic encoder for arquitectures described in ArchC
    Copyright (C) 2002-2004  The ArchC Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
*/

/********************************************************/
/* The ArchC Encoder Generator                          */
/* Author: Marcelo de Almeida Oliveira                  */
/* Modified by: Marcus Bartholomeu                      */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ac_encoder.H"


unsigned int pow2(int expoent) {
  return 1 << expoent;
}

void inttobin(unsigned int number, char * string, int size) {

  int pos = 0;

  while(pos < size) {
    if (number >= pow2(size-pos-1)) {
      string[pos] = '1';
      number = number - pow2(size-pos-1);
    } else {
      string[pos] = '0';
    }
    pos++;
  }

  string[size] = '\0';

  return;
}

void stringtohexa(char * string, char * hexa, int size)
{
  char aux[5];
  int b = 0, i = 0;
  
  #ifdef USE_LITTLE_ENDIAN
  for(b = (size-2)*4; b >= 0; b -= 8) 
  #else
  for(b = 0; b < 4*size; b += 8) 
  #endif  
    for(i = b; i < b+8; i += 4) {

      aux[0] = string[i];
      aux[1] = string[i+1];
      aux[2] = string[i+2];
      aux[3] = string[i+3];
      aux[4] = '\0';
      
      if (strstr(aux, "0000")) { hexa[i/4] = '0'; continue; }
      if (strstr(aux, "0001")) { hexa[i/4] = '1'; continue; }
      if (strstr(aux, "0010")) { hexa[i/4] = '2'; continue; }
      if (strstr(aux, "0011")) { hexa[i/4] = '3'; continue; }
      if (strstr(aux, "0100")) { hexa[i/4] = '4'; continue; }
      if (strstr(aux, "0101")) { hexa[i/4] = '5'; continue; }
      if (strstr(aux, "0110")) { hexa[i/4] = '6'; continue; }
      if (strstr(aux, "0111")) { hexa[i/4] = '7'; continue; }
      if (strstr(aux, "1000")) { hexa[i/4] = '8'; continue; }
      if (strstr(aux, "1001")) { hexa[i/4] = '9'; continue; }
      if (strstr(aux, "1010")) { hexa[i/4] = 'A'; continue; }
      if (strstr(aux, "1011")) { hexa[i/4] = 'B'; continue; }
      if (strstr(aux, "1100")) { hexa[i/4] = 'C'; continue; }
      if (strstr(aux, "1101")) { hexa[i/4] = 'D'; continue; }
      if (strstr(aux, "1110")) { hexa[i/4] = 'E'; continue; }
      if (strstr(aux, "1111")) { hexa[i/4] = 'F'; continue; }
    }
    
  hexa[size] = '\0';
}



void ac_encoder(int ac, char *av[], ac_decoder_full *decoder)
{
  // Begin of extra tools
  printf("\n");
  printf("ac_encoder tools:\n");
  printf("--tree       prints decoder tree\n");
  printf("--encode     encodes instructions into hexa format\n");
  printf("--simulate   simulates instructions created by encode\n");
  printf("\n");

//   ac_decoder_full *decoder = mips1.mips1_mc->ISA.decoder;

  for(int k = 0; k < ac; k++)
    if (strstr(av[k],"--tree"))
      ShowDecoder(decoder->decoder, 2);

  for(int k = 0; k < ac; k++)
    if (strstr(av[k],"--encode")) {
      int i, j;
      char input[65];

      int instructionType, fieldValue;
      unsigned int instruction;
      char binary[65];
      char hexa[18];

      int pc = 0;
      char pc_hexa[5];

      FILE *file;

      file = fopen("hexa.code", "w");
      if (file == NULL) {
        printf("Error while opening file.");
        exit(1);
      }

      while (true) {
        instruction = 0;

        printf("\nAvailable instruction types: \n");

        i = 0;
        do {
          j = 0;
          printf("(%d) Name: %s Size: %d \n", i, decoder->formats[i].name, decoder->formats[i].size);
          //printf("Fields: ");
          //do {
          //  printf("%s:%d:%d ", decoder->formats[i].fields[j].name, decoder->formats[i].fields[j].size, decoder->formats[i].fields[j].first_bit);
          //  j++;
          //} while (decoder->formats[i].fields[j-1].next != NULL);
          //printf("\n");
          i++;
        } while (decoder->formats[i-1].next != NULL);

        printf("\nChoose instruction type> ");
        scanf("%65s", &input);
        instructionType = atoi(input);

        printf("Chosen instruction type: %s \n", decoder->formats[instructionType].name);

        j = 0;
        do {
          printf("\nPlease enter in binary %s:%d value> ", decoder->formats[instructionType].fields[j].name, decoder->formats[instructionType].fields[j].size);
          scanf("%65s", &input);
          if ((strlen(input) != 0) &&
              (strlen(input) != (unsigned int) decoder->formats[instructionType].fields[j].size)) {
            printf("Incorrect number of bits for this field.");
            continue;
          }
          if (strlen(input) != 0) {
            fieldValue = strtol(input, NULL, 2);
            instruction += fieldValue << (decoder->formats[instructionType].size - decoder->formats[instructionType].fields[j].first_bit - 1);
          }
          j++;
        } while (decoder->formats[instructionType].fields[j-1].next != NULL);

        inttobin(instruction, binary, decoder->formats[instructionType].size);
        stringtohexa(binary, hexa, decoder->formats[instructionType].size/4);
        printf("\nEncoded instruction = %s = %u = %s\n", binary, instruction, hexa);

        inttobin(pc, binary, 16);
        stringtohexa(binary, pc_hexa, 4);
        pc += decoder->formats[instructionType].size/8;

        fprintf(file, "%s %s\n", pc_hexa, hexa);

        printf("\nPress enter for a new instruction or f to finish> ");
        scanf("%65s", &input);
        if (strstr(input,"f"))
          break;
      }

      fclose(file);
    }

  for(int k = 0; k < ac; k++)
    if (strstr(av[k],"--simulate"))
      av[1] = "--load=hexa.code";

  // End of extra tools

//   for(int k = 0; k < ac; k++)
//     fprintf(stderr, "%d %s \n", k, av[k]);
}
