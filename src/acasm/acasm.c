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
/* acasm.c: The ArchC assembler generator.              */
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

#include "ac_decoder.h"
#include "parser.acasm.h"
#include "acasm.h"
#include <getopt.h>


/* Opcodes library */
static int  CreateOpcHFile();
static int  CreateOpcCFile();

static void CreateOpcodeTable(FILE *output);
static void CreateAsmSymbolTable(FILE *output);
static void CreatePseudoOpsTable(FILE *output);

/* Bfd library */
static int  CreateBfdCpuFile();
static int  CreateBfdElfCFile();
static int  CreateBfdElfHFile();

/* Gas */
static int  CreateTcHFile();
static int  CreateTcSedFile();
static int  CreateTcCFile();

static void CreateEncodingFunc(FILE *output);
static void CreateGetFieldSizeFunc(FILE *output);


/* General internal error handling function */
static void internal_error() {
  printf("Internal Error. Contact the ArchC Team.\n");
  exit(-1);
}


/*
  General definitions
*/

/* Indentation */
#define IND1 "  "
#define IND2 "    "
#define IND3 "      "
#define IND4 "        "
#define IND5 "          "

/* Opcodes library variable names */ 
#define OPCODE_TABLE_VAR_DEF  "acasm_opcode opcodes[]"
#define NUM_OPCODES_VAR_DEF   "const int num_opcodes"
#define PSEUDO_TABLE_VAR_DEF  "const char *pseudo_instrs[]"
#define NUM_PSEUDOS_VAR_DEF   "const int num_pseudo_instrs"
#define ASM_SYMBOL_VAR_DEF    "const acasm_symbol udsymbols[]"
#define NUM_SYMBOLS_VAR_DEF   "const int num_symbols"

/* TC header function definitions */
#define ENCODE_INSN_FUNC_DEF "static unsigned long ac_encode_insn(unsigned long insn_type, int field_id, unsigned long value)"
#define GET_FIELD_SIZE_FUNC_DEF "static unsigned long get_field_size(unsigned long insn_fmt, int field_id)"


/* 
 Command line parsing stuff
*/
/* Show usage function (--help) */
static void show_usage(FILE * stream)
{
  fprintf(stream, "Usage:  acasm [options] file...\n");
  fprintf(stream, "Options:\n");

  fprintf(stream, "Mandatory:\n");
  fprintf(stream, "\
  -a, --arch=<arch_name>     Set the name of the architecture to be built \n\
                             to <arch_name>\n");

  fprintf(stream, "Optional:\n");
  fprintf(stream, "\
  -h, --help                 Display this information\n");
  fprintf(stream, "\
  -v, --version              DIsplay acasm version number\n");


  fprintf(stream, "\nReport bugs to ArchC Team: www.archc.org\n");
}

/* Show version number (--version) */
static void show_version(FILE *stream)
{
  fprintf(stream, "This is acasm version 1.5.1\n");
}

char *arch_name = NULL; /* name of the architecture */
char *file_name = NULL; /* name of the main ArchC file */

static const char *shortopts = "-a:hv";

static const struct option longopts[] =
{
  {"help",    no_argument,       NULL, 'h'},
  {"version", no_argument,       NULL, 'v'},
  {"arch",    required_argument, NULL, 'a'},
  {NULL,      no_argument,       NULL, 0}
};



/*
  Main Code

*/
int main(int argc, char **argv)
{

  /* got from architecture file by the parser */
  extern char *isa_filename;  

  /* Initializes the pre-processor */
  /* TODO: make acppInit() accept a parameter telling whether extended 
     set_asm parameter is to be recognized or not */
  extern int support_extended_setasm;
  support_extended_setasm = 1;
  acppInit();


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
      if (arch_name != NULL) {
	fprintf(stderr, "Argument duplicated: '%s'.\n", optarg);
	exit(1);
      }
      arch_name = (char *) malloc(strlen(optarg)+1);
      strcpy(arch_name, optarg);
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

  if (arch_name == NULL) {
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

  /* Opcodes files */
  if (!CreateOpcHFile()) {
    fprintf(stderr, "Error creating Opcodes H file.\n");
    exit(1);
  }        
  
  if (!CreateOpcCFile()) {
    fprintf(stderr, "Error creating Opcodes C file.\n");
    exit(1);
  }  


  /* BFD files */
  if (!CreateBfdCpuFile()) {
    fprintf(stderr, "Error creating BFD cpu file.\n");
    exit(1);
  }

  if (!CreateBfdElfCFile()) {
    fprintf(stderr, "Error creating BFD elf C file.\n");
    exit(1);
  }

  if (!CreateBfdElfHFile()) {
    fprintf(stderr, "Error creating BFD elf H file.\n");
    exit(1);
  }


  /* GAS files */
  if (!CreateTcHFile()) {
    fprintf(stderr, "Error creating GAS tc H file.\n");
    exit(1);
  }

  if (!CreateTcSedFile()) {
    fprintf(stderr, "Error creating SED file.\n");
    exit(1);
  }

  if (!CreateTcCFile()) {
    fprintf(stderr, "Error creating GAS tc C file.\n");
    exit(1);
  }

  return 0;
}





/***********************************************

OPCODES generated files

************************************************/

/* 
  Header file 

*/
static int CreateOpcHFile() {

  FILE *output;

  char buffer[100];

  strcpy(buffer, "binutils/include/opcode/");
  strcat(buffer, arch_name);
  strcat(buffer, ".h");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  fprintf(output, "#ifndef _OPC_H_FILE_\n");
  fprintf(output, "#define _OPC_H_FILE_\n");

  fprintf(output, "\n");

  /* opcode table structure */
  fprintf(output, "typedef struct {\n");
  fprintf(output, "%sconst char *mnemonic;\n", IND1);
  fprintf(output, "%sconst char *args;\n", IND1);
  fprintf(output, "%sunsigned long image;\n", IND1);
  fprintf(output, "%sunsigned long format_id;\n", IND1);
  fprintf(output, "%sunsigned long pseudo_idx;\n", IND1);
  fprintf(output, "%sunsigned long counter;\n", IND1);
  fprintf(output, "} acasm_opcode;\n");

  fprintf(output, "\n");

  /* user-defined symbol structure */
  fprintf(output, "typedef struct {\n");
  fprintf(output, "%sconst char *symbol;\n", IND1);
  fprintf(output, "%sconst char *cspec;\n", IND1);
  fprintf(output, "%sunsigned long value;\n", IND1);
  fprintf(output, "} acasm_symbol;\n");

  fprintf(output, "\n");

  /* variables exported */
  fprintf(output, "extern %s;\n", NUM_OPCODES_VAR_DEF);
  fprintf(output, "extern %s;\n", OPCODE_TABLE_VAR_DEF);
  fprintf(output, "extern %s;\n", ASM_SYMBOL_VAR_DEF);
  fprintf(output, "extern %s;\n", NUM_SYMBOLS_VAR_DEF);
  fprintf(output, "extern %s;\n", PSEUDO_TABLE_VAR_DEF);
  fprintf(output, "extern %s;\n", NUM_PSEUDOS_VAR_DEF);

  fprintf(output, "\n");

  fprintf(output, "#endif\n");

  fclose(output); 
  return 1;
}



/* 
  C file 

*/
static int CreateOpcCFile() {

  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/opcodes/");
  strcat(buffer, arch_name);
  strcat(buffer, "-opc.c");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  /* include files */
  fprintf(output, "\n#include <stdio.h>\n");
  fprintf(output, "#include \"sysdep.h\"\n");
  fprintf(output, "#include \"opcode/%s.h\"", arch_name);
  
  fprintf(output, "\n\n");
  CreateOpcodeTable(output);     /* write the opcode table */
  
  fprintf(output, "\n");
  CreateAsmSymbolTable(output);  /* write the symbol table */
  
  fprintf(output, "\n");
  CreatePseudoOpsTable(output);  /* write the pseudo-op table */

  fprintf(output, "\n");

  /* number of opcodes */
  fprintf(output, "\n%s = ((sizeof opcodes) / (sizeof(opcodes[0])));\n", NUM_OPCODES_VAR_DEF);

  /* number of symbols */
  fprintf(output, "%s = ((sizeof udsymbols) / (sizeof(udsymbols[0])));\n", NUM_SYMBOLS_VAR_DEF);

  /* number of pseudos */
  fprintf(output, "%s = ((sizeof pseudo_instrs) / (sizeof(pseudo_instrs[0])));\n", NUM_PSEUDOS_VAR_DEF);

  fclose(output);
  return 1;
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
static void CreateOpcodeTable(FILE *output)
{
  char *strP;

  /* get the instruction list from parser */
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();


  /* table name */
  fprintf(output, "%s = {\n", OPCODE_TABLE_VAR_DEF);

  long pseudo_idx = 1;  /* index to an instruction in the pseudo_instr table */  
  while (asml != NULL) {

    /* mnemonic */
    /*----------------------------------------------------------------*/
    fprintf(output, "%s{\"%s\",\t", IND1, asml->mnemonic);
    /*----------------------------------------------------------------*/

    /* args string */
    /*----------------------------------------------------------------*/
    fprintf(output, "\"%s\",\t", asml->operand);
    /*----------------------------------------------------------------*/
    
    /* base image */
    /*----------------------------------------------------------------*/
    unsigned long base_image = 0x00;
    unsigned long format_id = 99;     /* a pseudo_instr has a 99 format id */
    extern ac_dec_format *format_ins_list;
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
	    else base_image |= ac_asm_encode_insn_field(pdl->value, pfrm, pdf);
	      	 
	    pdl = pdl->next;	    
	  }	  
	}

	base_image |= asml->const_image;	
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
  
  fprintf(output, "};\n"); 
}



/*
  User defined symbol table generation

  TODO: describe the acasm_symbol structure

*/
static void CreateAsmSymbolTable(FILE *output) {

  fprintf(output, "%s = {\n", ASM_SYMBOL_VAR_DEF);

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

  fprintf(output, "};\n");
}


/*
  Pseudo instruction table generation

  TODO: describe the structure

*/
static void CreatePseudoOpsTable(FILE *output) {

  /* gets the list from the parser */
  ac_asm_insn *asml = ac_asm_get_asm_insn_list();

  fprintf(output, "%s = {\n", PSEUDO_TABLE_VAR_DEF);

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

  fprintf(output, "\n};\n");
}




/***********************************************

BFD generated files

************************************************/

/*
  CPU file

*/
static int CreateBfdCpuFile() {
  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/bfd/cpu-");
  strcat(buffer, arch_name);
  strcat(buffer, ".c");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  /* include files */
  fprintf(output, "\n#include \"bfd.h\"\n");
  fprintf(output, "#include \"sysdep.h\"\n");
  fprintf(output, "#include \"libbfd.h\"\n");

  fprintf(output, "\n");

  /* the structure */
  fprintf(output, "const bfd_arch_info_type bfd_%s_arch = {\n", arch_name);
  fprintf(output, "%s32,\n", IND1); /* bits in a word */
  fprintf(output, "%s32,\n", IND1); /* bits in an address */
  fprintf(output, "%s8,\n", IND1);
  fprintf(output, "%sbfd_arch_%s,\n", IND1, arch_name); 
  fprintf(output, "%s0,\n", IND1);
  fprintf(output, "%s\"%s\",\n", IND1, arch_name);
  fprintf(output, "%s\"%s\",\n", IND1, arch_name);
  fprintf(output, "%s3,\n", IND1); /* section align power */
  fprintf(output, "%sTRUE,\n", IND1);
  fprintf(output, "%sbfd_default_compatible,\n", IND1);
  fprintf(output, "%sbfd_default_scan,\n", IND1);
  fprintf(output, "%sNULL\n", IND1);

  fprintf(output, "};\n");

  fclose(output);
  return 1;
}

/*
  Elf H File
*/
static int CreateBfdElfHFile() {
  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/include/elf/");
  strcat(buffer, arch_name);
  strcat(buffer, ".h");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  fprintf(output, "#ifndef _ELF%s_H_FILE_\n", arch_name);
  fprintf(output, "#define _ELF%s_H_FILE_\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "#endif\n");

  fclose(output);
  return 1;
}

/*
  Elf C file
*/
static int CreateBfdElfCFile() {
  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/bfd/elf32-");
  strcat(buffer, arch_name);
  strcat(buffer, ".c");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  /* include files */
  fprintf(output, "\n#include \"bfd.h\"\n");
  fprintf(output, "#include \"sysdep.h\"\n");
  fprintf(output, "#include \"libbfd.h\"\n");
  fprintf(output, "#include \"elf-bfd.h\"\n");
  fprintf(output, "#include \"elf/%s.h\"\n", arch_name);

  fprintf(output, "\n");

  /* Local label (those who starts with $ are recognized as well) */
  fprintf(output, "static bfd_boolean\n");
  fprintf(output, "%s_elf_is_local_label_name (bfd *abfd, const char *name)\n", arch_name);
  fprintf(output, "{\n");
  fprintf(output, "%sif (name[0] == \'$\')\n", IND1);
  fprintf(output, "%sreturn TRUE;\n", IND2);
  fprintf(output, "\n");
  fprintf(output, "%sreturn _bfd_elf_is_local_label_name(abfd, name);\n", IND1);
  fprintf(output, "}\n");

  fprintf(output, "\n");

  fprintf(output, "#define TARGET_BIG_SYM                    bfd_elf32_%s_vec\n", arch_name);
  fprintf(output, "#define TARGET_BIG_NAME                   \"elf32-%s\"\n", arch_name);
  fprintf(output, "#define ELF_ARCH                          bfd_arch_%s\n", arch_name);
  fprintf(output, "#define ELF_MACHINE_CODE                  EM_NONE\n");
  fprintf(output, "#define ELF_MAXPAGESIZE                   0x1\n");
  fprintf(output, "#define bfd_elf32_bfd_reloc_type_lookup   bfd_default_reloc_type_lookup\n");
  fprintf(output, "#define bfd_elf32_bfd_is_local_label_name %s_elf_is_local_label_name\n");

  fprintf(output, "\n#include \"elf32-target.h\"\n");

  fclose(output);
  return 1;
}





/***********************************************

GAS generated files

************************************************/

/*
  Tc H file
*/
static int CreateTcHFile() {
  FILE *output;
  char buffer[100];

  extern int ac_tgt_endian; /* filled by the parser 0=l, 1=b */

  strcpy(buffer, "binutils/gas/config/tc-");
  strcat(buffer, arch_name);
  strcat(buffer, ".h");  

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  fprintf(output, "#ifndef _TC_%s_H_FILE_\n", arch_name);
  fprintf(output, "#define _TC_%s_H_FILE_\n", arch_name);

  fprintf(output, "\n");

  if (ac_tgt_endian) 
    fprintf(output, "#define AC_BIG_ENDIAN\n");

  fprintf(output, "#define AC_WORD_SIZE %d\n", 32);
  fprintf(output, "#define TARGET_BYTES_BIG_ENDIAN %d\n", ac_tgt_endian);

  fprintf(output, "#define TARGET_ARCH bfd_arch_%s\n", arch_name);
  fprintf(output, "#define TARGET_FORMAT \"elf32-%s\"\n", arch_name);
  fprintf(output, "#define WORKING_DOT_WORD\n");
  fprintf(output, "#define LOCAL_LABELS_FB 1\n");
  
  fprintf(output, "\n");

  fprintf(output, "typedef struct fix_addend {\n");
  fprintf(output, "%sunsigned long insn_format_id;\n", IND1);
  fprintf(output, "%sunsigned long insn_field_id;\n", IND1);
  fprintf(output, "%sint al_value;\n", IND1);
  fprintf(output, "%sint hl_value;\n", IND1);

  fprintf(output, "%sint flag;\n", IND1);
  fprintf(output, "%sint field_size;\n", IND1);
  fprintf(output, "%sint pcrel_add;\n", IND1);
  fprintf(output, "} archc_fix_addend;\n");

  fprintf(output, "\n");

  fprintf(output, "#define TC_FIX_TYPE archc_fix_addend\n");
  fprintf(output, "#define TC_INIT_FIX_DATA(fixP)\n");

  fprintf(output, "\n");

  fprintf(output, "extern int %s_parse_name(char *name, expressionS *expP, char *c);\n", arch_name);
  fprintf(output, "#define md_parse_name(x, y, z) %s_parse_name(x, y, z)\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "extern void %s_frob_label(symbolS *sym);\n", arch_name);
  fprintf(output, "#define tc_frob_label(x) %s_frob_label(x)\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "extern void %s_handle_align(struct frag *);\n", arch_name);
  fprintf(output, "#define HANDLE_ALIGN(fragp) %s_handle_align(fragp)\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "extern void %s_cons_fix_new(struct frag *, int, unsigned int, struct expressionS *);\n", arch_name);
  fprintf(output, "#define TC_CONS_FIX_NEW %s_cons_fix_new\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "#define TC_VALIDATE_FIX(fixp, this_segment, skip_label) \\\n");
  fprintf(output, "%sdo \\\n", IND1);
  fprintf(output, "%sif (!%s_validate_fix((fixp), (this_segment))) \\\n", IND2, arch_name);
  fprintf(output, "%sgoto skip_label; \\\n", IND3);
  fprintf(output, "%swhile (0)\n", IND1);
  fprintf(output, "extern int %s_validate_fix(struct fix *, asection *);\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "extern void %s_elf_final_processing PARAMS ((void));\n", arch_name);
  fprintf(output, "#define elf_tc_final_processing %s_elf_final_processing\n", arch_name);

  fprintf(output, "\n");

  fprintf(output, "#define DIFF_EXPR_OK            /* foo-. gets turned into PC relative relocs */");

  fprintf(output, "\n");
  fprintf(output, "#endif\n");

  fclose(output);
  return 1;
}

/*
  Tc sed file
*/
static int CreateTcSedFile() {
  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/gas/config/tc-templ.sed");

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  fprintf(output, "/this comment is used by sed/ a\\\n");
  fprintf(output, "%s; \\\n", ENCODE_INSN_FUNC_DEF);
  fprintf(output, "%s;\n", GET_FIELD_SIZE_FUNC_DEF);

  fclose(output);
  return 1;

}

/*
  Tc C file
*/
static int CreateTcCFile() {
  FILE *output;
  char buffer[100];

  strcpy(buffer, "binutils/gas/config/tc-funcs.c");

  if ((output = fopen(buffer, "w")) == NULL) 
    return 0;

  CreateEncodingFunc(output);
  CreateGetFieldSizeFunc(output);

  fclose(output);
  return 1;
}


static void CreateEncodingFunc(FILE *output)
{
  /* function prototype and local data */
  fprintf(output, "\n%s {\n", ENCODE_INSN_FUNC_DEF);
  fprintf(output, "%sunsigned long mask1 = 0xffffffff;\n", IND1);
  fprintf(output, "%sunsigned long mask2 = 0xffffffff;\n\n", IND1);

  /* instruction format type switch */
  fprintf(output, "%sswitch(insn_type) {\n", IND1);
 
  extern ac_dec_format *format_ins_list;
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
      fprintf(output, "%smask1 <<= %i;\n", IND4, pfrm->size - (pf->first_bit+1));
      fprintf(output, "%smask2 >>= %i;\n", IND4, pf->first_bit+1 - pf->size);
      fprintf(output, "%sreturn (value << (%i)) & (mask1 & mask2);\n\n", IND4, pfrm->size - (pf->first_bit+1));
      
      pf = pf->next;
      j++;
    }
    fprintf(output, "%s}\n", IND3);
 
    fprintf(output, "\n%sbreak;\n", IND3);
    pfrm = pfrm->next;
    i++;
  }
  fprintf(output, "\n%s}\n", IND1);
 
  fprintf(output, "%sreturn 0;\n", IND1);

  fprintf(output, "\n}\n");
}



static void CreateGetFieldSizeFunc(FILE *output)
{
  /* function prototype and local data */
  fprintf(output, "\n%s {\n", GET_FIELD_SIZE_FUNC_DEF);

  /* instruction type switch */
  fprintf(output, "%sswitch(insn_fmt) {\n", IND1);
 
  extern ac_dec_format *format_ins_list;
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

  fprintf(output, "%sreturn 0;\n", IND1);

  fprintf(output, "\n}\n");
}
