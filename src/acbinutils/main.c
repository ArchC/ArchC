/* ex: set tabstop=2 expandtab: */
/* 
    ArchC assembler generator - Copyright (C) 2002-2005  The ArchC Team

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
/* main.c: The ArchC binary utilities generator.        */
/* Author: Alexandro Baldassin                          */
/* Date: 01-06-2005                                     */
/*                                                      */
/* The ArchC Team                                       */
/* Computer Systems Laboratory (LSC)                    */
/* IC-UNICAMP                                           */
/* http://www.lsc.ic.unicamp.br                         */
/********************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include "utils.h"
#include "opcodes.h"
#include "bfd.h"
#include "gas.h"

static int Createm4File();





/*
 * File names definition
 */
#define OPCODE_TABLE_FILE  "opcode.table"
#define SYMBOL_TABLE_FILE  "symbol.table"
#define PSEUDO_TABLE_FILE  "pseudo.table"
#define RELOC_IDS_FILE     "reloc.ids"
#define RELOC_HOWTO_FILE   "reloc.howto"
#define RELOC_MAP_FILE     "reloc.map"
#define ENCODING_FN_FILE   "encoding.fn"
#define FIELD_SIZE_FN_FILE "fieldsize.fn"
#define INSN_SIZE_FN_FILE  "insnsize.fn"

#define GEN_DIR "acbingenbuilddir/"


/* 
 Command line parsing stuff
*/
/* Show usage function (--help) */
static void show_usage(FILE * stream)
{
  fprintf(stream, "This is bmdsfg, the Binutils Machine-Dependent Source File Generator.\n\n");
  fprintf(stream, "Usage: bmdsfg [options] file...\n");
  fprintf(stream, "Options:\n");

  fprintf(stream, "Mandatory:\n");
  fprintf(stream, "\
  -a, --arch=<arch_name>     Set the name of the architecture to be \n\
                             built to <arch_name>\n");

  fprintf(stream, "Optional:\n");
  fprintf(stream, "\
  -h, --help                 Display this information\n");
  fprintf(stream, "\
  -v, --version              Display bmdsfg version number\n");


  fprintf(stream, "\nReport bugs to ArchC Team: www.archc.org\n");
}

/* Show version number (--version) */
static void show_version(FILE *stream)
{
  fprintf(stream, "This is bmdsfg version %s\n", ACVERSION);
}


static const char *shortopts = "-a:hv";

static const struct option longopts[] =
{
  {"help",    no_argument,       NULL, 'h'},
  {"version", no_argument,       NULL, 'v'},
  {"arch",    required_argument, NULL, 'a'},
  {NULL,      no_argument,       NULL, 0}
};


char *file_name = NULL; /* name of the main ArchC file */

/*
  Main Code

*/
int main(int argc, char **argv)
{
  char buffer[200];
  
  /* Initializes the pre-processor */
  acppInit(1);

  /* Command line parsing code */
  while (1) {

    int longind;
    int optc = getopt_long_only(argc, argv, shortopts, longopts,
                &longind);

    if (optc == -1)
      break;
    
    switch (optc) {

    case 1:  /* file */
      if (file_name != NULL) {
        fprintf(stderr, "Argument duplicated: '%s'.\n", optarg);
        exit(1);
      }
      file_name = (char *) malloc(strlen(optarg)+1);
      strcpy(file_name, optarg);
      break;

    case 'a':
      if (get_arch_name() != NULL) {
        fprintf(stderr, "Argument duplicated: '%s'.\n", optarg);
        exit(1);
      }
      set_arch_name(optarg);
      break;

    case 'h':
      show_usage(stdout);
      exit(0);

    case 'v':
      show_version(stdout);
      exit(0);

    default:
      show_usage(stdout);
      exit(1);
    }
  }

  if (file_name == NULL) {
    fprintf(stderr, "No ArchC description file specified.\n");
    exit(1);
  }

  if (get_arch_name() == NULL) {
    fprintf(stderr, "No architecture name specified.\n");
    exit(1);
  }

  /* Parse the ARCH file */
  if (!acppLoad(file_name)) {
    fprintf(stderr, "Invalid file: '%s'.\n", file_name);
    exit(1);
  }

  if (acppRun()) {
    fprintf(stderr, "Parser error in ARCH file.\n");
    exit(1);
  }
  acppUnload();


  /* Parse the ISA file */
  if (!acppLoad(isa_filename)) {
    fprintf(stderr, "Invalid ISA file: '%s'.\n", isa_filename);
    exit(1);
  }
  if (acppRun()) {
    fprintf(stderr, "Parser error in ISA file.\n");
    exit(1);
  }
  acppUnload();


  /*
    File Generation
  */
 
  if (!Createm4File()) {
    fprintf(stderr, "Error creating m4 file.\n");
    exit(1);
  }
 
  // Create the relocation list first
  create_relocation_list();

  strcpy(buffer, GEN_DIR);
  strcat(buffer, OPCODE_TABLE_FILE);
  if (!CreateOpcodeTable(buffer)) {     /* write the opcode table */
    fprintf(stderr, "Error creating opcode table.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, SYMBOL_TABLE_FILE);  
  if (!CreateAsmSymbolTable(buffer)) {  /* write the symbol table */
    fprintf(stderr, "Error creating symbol table.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, PSEUDO_TABLE_FILE);
  if (!CreatePseudoOpsTable(buffer)) {  /* write the pseudo-op table */
    fprintf(stderr, "Error creating pseudo table.\n");
    exit(1);
  }
  
  strcpy(buffer, GEN_DIR);
  strcat(buffer, RELOC_IDS_FILE);
  if (!CreateRelocIds(buffer)) {
    fprintf(stderr, "Error creating relocation IDs.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, RELOC_HOWTO_FILE);
  if (!CreateRelocHowto(buffer)) {
    fprintf(stderr, "Error creating relocation HOWTO structure.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, RELOC_MAP_FILE);
  if (!CreateRelocMap(buffer)) {
    fprintf(stderr, "Error creating relocation map.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, ENCODING_FN_FILE);
  if (!CreateEncodingFunc(buffer)) {
    fprintf(stderr, "Error creating encoding function.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, FIELD_SIZE_FN_FILE);
  if (!CreateGetFieldSizeFunc(buffer)) {
    fprintf(stderr, "Error creating field size function.\n");
    exit(1);
  }

  strcpy(buffer, GEN_DIR);
  strcat(buffer, INSN_SIZE_FN_FILE);
  if (!CreateGetInsnSizeFunc(buffer)) {
    fprintf(stderr, "Error creating insn size function.\n");
    exit(1);
  }
  

  return 0;
}


static int Createm4File() 
{
  FILE *output;

  if ((output = fopen("acbingenbuilddir/defines.m4", "w")) == NULL) 
    return 0;

  /* disable comments */
  fprintf(output, "m4_changecom()m4_dnl\n");
  
  fprintf(output, "m4_define(`___arch_name___', `%s')m4_dnl\n", get_arch_name());
  fprintf(output, "m4_define(`___word_size___', `%d')m4_dnl\n", get_arch_size());

  /* 1 = big, 0 = little */
  fprintf(output, "m4_define(`___endian_str___', `%s')m4_dnl\n", ac_tgt_endian ? "AC_BIG_ENDIAN" : "AC_LITTLE_ENDIAN");
  fprintf(output, "m4_define(`___endian_val___', `%d')m4_dnl\n", ac_tgt_endian ? 1 : 0);


  fprintf(output, "m4_define(`___opcode_table___', `m4_include(%s)')m4_dnl\n", OPCODE_TABLE_FILE);
  fprintf(output, "m4_define(`___symbol_table___', `m4_include(%s)')m4_dnl\n", SYMBOL_TABLE_FILE);
  fprintf(output, "m4_define(`___pseudo_table___', `m4_include(%s)')m4_dnl\n", PSEUDO_TABLE_FILE);

  
  fprintf(output, "m4_define(`___reloc_ids___', `m4_include(%s)')m4_dnl\n", RELOC_IDS_FILE);
  fprintf(output, "m4_define(`___reloc_howto___', `m4_include(%s)')m4_dnl\n", RELOC_HOWTO_FILE);
  fprintf(output, "m4_define(`___reloc_map___', `m4_include(%s)')m4_dnl\n", RELOC_MAP_FILE);


  fprintf(output, "m4_define(`___encoding_function___', `m4_include(%s)')m4_dnl\n", ENCODING_FN_FILE);
  fprintf(output, "m4_define(`___fieldsize_function___', `m4_include(%s)')m4_dnl\n", FIELD_SIZE_FN_FILE);
  fprintf(output, "m4_define(`___insnsize_function___', `m4_include(%s)')m4_dnl\n", INSN_SIZE_FN_FILE);
  
  fclose(output);

  return 1;
}
