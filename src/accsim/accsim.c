/* -*- Mode: C; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      accsim.c
 * @author    Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Wed, 19 Mar 2003 08:07:46 -0200
 * 
 * @brief     The ArchC pre-processor for compiled simulation.
 *            This file contains functions for compiled simulation.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "acsim.h"
#include "accsim.h"
#include "ac_decoder.h"

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include <errno.h>
#include "eval.h"

char *data_mem=0;
unsigned int data_mem_size=0;
unsigned int data_mem_start=0;
unsigned int ac_heap_ptr=0;
unsigned int ac_start_addr=0;
int prog_size_bytes=0;
int prog_size_instr=0;
unsigned char *instr_mem=0;
unsigned int prog_entry_point=0;
extern char *project_name;
extern ac_decoder_full *decoder;
extern int ACABIFlag;
extern int ACStatsFlag;
extern int ACDelayFlag;

extern unsigned eval_result;
extern char* eval_input;

#include "accsim_extras.h"


//Prototype
ac_sto_list *accs_FindLoadDevice();


/*************************************************************************************/
/*! Function for decoder that updates the the bytes quantity available in the buffer */
unsigned char *fetch;
//char buffer[50];
int quant;

unsigned long long GetWord(int index)
{
  extern int wordsize;
  extern int ac_host_endian;
  
  int wordbytes = wordsize/8;
  int i;
  unsigned long long word=0;

    // Test if host is little-endian
    if (ac_host_endian == 0) {
      for (i=wordbytes-1; i>=0; i--) {
        word <<= 8;
        word |= fetch[index*wordbytes+i];
      }
    }

    // else host is big-endian
    else {
      for (i=0; i<wordbytes; i++) {
        word <<= 8;
        word |= fetch[index*wordbytes+i];
      }
    }

  return word;
};

#define DECODE(addr) (fetch=addr, quant=100, Decode(decoder,fetch,quant))
/*************************************************************************************/

/**********************************************************************/
/*! Function for decoder that gets the requested bits from the buffer */
//TODO: buffer type should be ac_word, but how to do it in parser time?
  unsigned long long GetBits(int *buffer, int *quant, int last, int quantity, int sign)
  {
    //! Read the buffer using this macro
//#define BUFFER(index) ((index<*quant) ? ( ((int*)buffer)[index]) : (*quant=ExpandInstrBuffer(index),((int*)buffer)[index]))
//#define BUFFER(index) ( ((int*)fetch)[index])

    extern int wordsize;
    extern int ac_tgt_endian;

    int first = last - (quantity-1);

    int index_first = first/wordsize;
    int index_last = last/wordsize;

    unsigned long long value = 0;
    int i;

//#if AC_PROC_ENDIAN == 1   /* if processor is big-endian */
    if (ac_tgt_endian == 1) {

    //big-endian: first  last
    //          0xAA BB CC DD

    //Read words from first to last
    for (i=index_first; i<=index_last; i++) {
      value <<= wordsize;
      value |= GetWord(i);
    }

    //Remove bits before last
    value >>= wordsize - (last%wordsize + 1);

//#else
    } else {

    //little-endian: last  first
    //             0xAA BB CC DD

    //Read words from last to first
    for (i=index_last; i>=index_first; i--) {
      value <<= wordsize;
      value |= GetWord(i);
    }

    //Remove bits before first
    value >>= first%wordsize;

//#endif
    }

    //Mask to the size of the field
    value &= (~((~0LL) << quantity));

    //If signed, sign extend if necessary
    if (sign && ( value >= (1 << (quantity-1)) ))
      value |= (~0LL) << quantity;

    return value;
#undef BUFFER
  }
/*************************************************************************************/


int accs_main()
{
  extern int ACCompsimFlag;
  extern char *ACCompsimProg;
  extern int ControlInstrInfoLevel;
  extern int stage_num;
  extern int HaveMultiCycleIns;
  extern int HaveFormattedRegs;

  //Assert there is no pipeline stages
  if (stage_num > 0) {
    AC_ERROR("pipelined models are not supported in compiled simulation at this time, sorry.\n");
    exit(EXIT_FAILURE);
  }

  //Assert there is no multicycle
  if (HaveMultiCycleIns) {
    AC_ERROR("multicycle instructions are not supported in compiled simulation at this time, sorry.\n");
    exit(EXIT_FAILURE);
  }

  //Test if the optimization required is available
  if (PROCESSOR_OPTIMIZATIONS > ControlInstrInfoLevel) {
    PROCESSOR_OPTIMIZATIONS = ControlInstrInfoLevel;
    AC_MSG("WARNING: falling down to optimization level %d due to lack of information in the model.\n", PROCESSOR_OPTIMIZATIONS);
  }

  //Test if we can use the best optimization
  else if (PROCESSOR_OPTIMIZATIONS == -1) {
    extern int ACInlineFlag;
    PROCESSOR_OPTIMIZATIONS = ControlInstrInfoLevel;
    AC_MSG("Using the best optimization available from model information (level %d).\n", PROCESSOR_OPTIMIZATIONS);
    if (!ACInlineFlag) AC_MSG("INFO: Try the \"--inline\" option together with the \"-opt\" for faster simulation (but slower compilation).\n");
  }

  //Inform we are using the requested optimization
  else if (PROCESSOR_OPTIMIZATIONS != 0){
    extern int ACInlineFlag;
    AC_MSG("Using optimization level %d.\n", PROCESSOR_OPTIMIZATIONS);
    if (!ACInlineFlag) AC_MSG("INFO: Try the \"--inline\" option together with the \"-opt\" for faster simulation (but slower compilation).\n");
  }

  //Read program file, trying ELF first
  if ((accs_ReadElf(ACCompsimProg) == EXIT_FAILURE) &&
      (accs_ReadHexProgram(ACCompsimProg) == EXIT_FAILURE) &&
      (accs_ReadObjdumpHex(ACCompsimProg) == EXIT_FAILURE)) {
    AC_ERROR("Aplication program %s: Format not recognized (tried ELF and HEX)\n", ACCompsimProg);
    exit(EXIT_FAILURE);
  }


  //
  //Create files used for all compiled simulation versions
  //

  //Creating Main File
  accs_CreateMain();

  //Creating Compiled Simulation Header File
  accs_CreateCompsimHeader();
  //Creating Compiled Simulation Implementation File
  accs_CreateCompsimImpl();
  //Creating Compiled Simulation Header for Region functions
  accs_CreateCompsimHeaderRegions();
  
  //Creating ISA Header File
  accs_CreateISAHeader();
  //Create the template for the .cpp instruction and format behavior file
  CreateImplTmpl();
  //Creating ISA Init Implementation File
  accs_CreateISAInitImpl();

  //Creating Resources Header File
  CreateResourceHeader();
  //Creating Resources Implementation File
  CreateResourceImpl();

  //Creating Program Memory Header File
  accs_CreateProgramMemoryHeader();
  //Creating Program Memory Implementation File
  accs_CreateProgramMemoryImpl();

  //Creating Architecture Dependent Syscalls Header File
  if (ACABIFlag)
    accs_CreateArchSyscallHeader();

  //Creating Statistics Header File
  if (ACStatsFlag)
    CreateStatsHeader();

  if( HaveFormattedRegs ){
    //Creating Formatted Registers Header and Implementation Files.
    CreateRegsHeader();
    //CreateRegsImpl();  This is not used anymore.
  }

  //!Creating Parameters Header File --- defined for interpreted simulation
  CreateParmHeader();

  //!Creating Makefile
  CreateMakefile();

  //!Creating dummy functions to use if real ones are undefined
  CreateDummy(1, "#include \"ac_parms.H\"\nnamespace ac_begin {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0){};}");
  CreateDummy(2, "#include \"ac_parms.H\"\nnamespace ac_end {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0){};}");


  // There are 2 methods for storage and resources:
  // 1) use special template classes for storage and namespace resources.
  //    (this is faster, but still cannot have formatted registers)
  // 2) use default ArchC provided ac_storage and ac_resources classes.
  //    (compatible with formatted registers, but slower and does not support delay assignment (with is implemented in systemc))
  //
  // Method 1 is default. To select method 2 use variation "-l 2 <filename>" in the command line

  if (ACCompsimFlag == 1) {

    //Creating Storage Header File
    fast_CreateStorageHeader();
    //Creating Storage Implementation File
    fast_CreateStorageImpl();

    //Creating Resources Header File
    fast_CreateResourcesHeader();
    //Creating Resources Implementation File
    fast_CreateResourcesImpl();

  }

  AC_MSG("%s model files generated for compiled simulation with program %s.\n", project_name, ACCompsimProg);
  return 0;
}


unsigned int convert_endian(unsigned int size, unsigned int num)
{
  extern int ac_match_endian;
  unsigned char *in = (unsigned char*) &num;
  unsigned int out = 0;

  if (! ac_match_endian) {
    for(; size>0; size--) {
      out <<= 8;
      out |= in[0];
      in++;
    }
  }
  else {
    out = num;
  }

  return out;
}


int ac_load_elf(char* filename, char* data_mem, unsigned int data_mem_size);


int accs_ReadElf(char* elffile) {
  //return accs_ReadElf_2(elffile);
  extern char* data_mem;
  extern unsigned char* instr_mem;
  extern unsigned int data_mem_size;

  data_mem_size = accs_FindLoadDevice()->size;
  data_mem = malloc(data_mem_size);
  instr_mem = data_mem;

  return ac_load_elf(elffile, data_mem, data_mem_size);
}


//Taken from archc.cpp
//  -- corrected AC_ERROR and AC_SAY to become C construct (AC_SAY -> AC_MSG)
//  -- sizeof(ac_fetch) is fixed with 4 (32 bits ELF), type is "unsigned int"
//  -- included code: Search for executable sections

//Loading binary application
int ac_load_elf(char* filename, char* data_mem, unsigned int data_mem_size)
{
  Elf32_Ehdr    ehdr;
  Elf32_Shdr    shdr;
  Elf32_Phdr    phdr;
  int           fd;
  unsigned int  i;

  //Open application
  if (!filename || ((fd = open(filename, 0)) == -1)) {
    AC_ERROR("Openning application file '%s': %s\n", filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Test if it's an ELF file
  if ((read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) ||  // read header
      (strncmp((char *)ehdr.e_ident, ELFMAG, 4) != 0) ||          // test elf magic number
      0) {
    close(fd);
    return EXIT_FAILURE;
  }
  
  //Set start address
  ac_start_addr = convert_endian(4,ehdr.e_entry);
  if (ac_start_addr > data_mem_size) {
    AC_ERROR("the start address of the application is beyond model memory\n");
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (convert_endian(2,ehdr.e_type) == ET_EXEC) {
    //It is an ELF file
    AC_MSG("Reading ELF application file: %s\n", filename);

    //Get program headers and load segments
    //    lseek(fd, convert_endian(4,ehdr.e_phoff), SEEK_SET);
    for (i=0; i<convert_endian(2,ehdr.e_phnum); i++) {

      //Get program headers and load segments
      lseek(fd, convert_endian(4,ehdr.e_phoff) + convert_endian(2,ehdr.e_phentsize) * i, SEEK_SET);
      if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
        AC_ERROR("reading ELF program header\n");
        close(fd);
        exit(EXIT_FAILURE);
      }

      if (convert_endian(4,phdr.p_type) == PT_LOAD) {
        Elf32_Word j;
        Elf32_Addr p_vaddr = convert_endian(4,phdr.p_vaddr);
        Elf32_Word p_memsz = convert_endian(4,phdr.p_memsz);
        Elf32_Word p_filesz = convert_endian(4,phdr.p_filesz);
        Elf32_Off  p_offset = convert_endian(4,phdr.p_offset);

        //Error if segment greater then memory
        if (data_mem_size < p_vaddr + p_memsz) {
          AC_ERROR("not enough memory in ArchC model to load application.\n");
          close(fd);
          exit(EXIT_FAILURE);
        }

        //Set heap to the end of the segment
        if (ac_heap_ptr < p_vaddr + p_memsz) ac_heap_ptr = p_vaddr + p_memsz;

        //Load and correct endian
        lseek(fd, p_offset, SEEK_SET);
        for (j=0; j < p_filesz; j+=4) {
          int tmp;
          read(fd, &tmp, 4);
          *(unsigned int *) (data_mem + p_vaddr + j) = convert_endian(4, tmp);
        }
        memset(data_mem + p_vaddr + p_filesz, 0, p_memsz - p_filesz);
      }

      //next header/segment
      //      lseek(fd, convert_endian(4,ehdr.e_phoff) + convert_endian(2,ehdr.e_phentsize) * i, SEEK_SET);
    }
  }
  else if (convert_endian(2,ehdr.e_type) == ET_REL) {

    AC_MSG("Reading ELF relocatable file: %s\n", filename);

    // first load the section name string table
    char *string_table;
    int   shoff = convert_endian(4,ehdr.e_shoff);
    short shndx = convert_endian(2,ehdr.e_shstrndx);
    short shsize = convert_endian(2,ehdr.e_shentsize);

    lseek(fd, shoff+(shndx*shsize), SEEK_SET);
    if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
      AC_ERROR("reading ELF section header\n");
      close(fd);
      exit(EXIT_FAILURE);
    }
    
    string_table = (char *) malloc(convert_endian(4,shdr.sh_size));
    lseek(fd, convert_endian(4,shdr.sh_offset), SEEK_SET);
    read(fd, string_table, convert_endian(4,shdr.sh_size));

    // load .text, .data and .bss sections    
    for (i=0; i<convert_endian(2,ehdr.e_shnum); i++) {

      lseek(fd, shoff + shsize*i, SEEK_SET);
      
      if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
        AC_ERROR("reading ELF section header\n");
        close(fd);
        exit(EXIT_FAILURE);
      }


      if (!strcmp(string_table+convert_endian(4,shdr.sh_name), ".text") ||
          !strcmp(string_table+convert_endian(4,shdr.sh_name), ".data") ||
          !strcmp(string_table+convert_endian(4,shdr.sh_name), ".bss")) {
        
        //        printf("Section %s:\n", string_table+convert_endian(4,shdr.sh_name));

        Elf32_Off  tshoff  = convert_endian(4,shdr.sh_offset);
        Elf32_Word tshsize = convert_endian(4,shdr.sh_size);
        Elf32_Addr tshaddr = convert_endian(4,shdr.sh_addr);

        if (tshsize == 0) {
          // printf("--- empty ---\n");
          continue;
        }

        if (data_mem_size < tshaddr + tshsize) {
          AC_ERROR("not enough memory in ArchC model to load application.\n");
          close(fd);
          exit(EXIT_FAILURE);
        }

        //Set heap to the end of the segment
        if (ac_heap_ptr < tshaddr + tshsize) ac_heap_ptr = tshaddr + tshsize;

        if (!strcmp(string_table+convert_endian(4,shdr.sh_name), ".bss")) {
          memset(data_mem + tshaddr, 0, tshsize);
          //continue;
          break; // .bss is supposed to be the last one
        }

        //Load and correct endian
        lseek(fd, tshoff, SEEK_SET);
        Elf32_Word j;
        for (j=0; j < tshsize; j+=4) {
          int tmp;
          read(fd, &tmp, 4);
          *(unsigned int *) (data_mem + tshaddr + j) = convert_endian(4, tmp);

          // printf("0x%08x %08x \n", tshaddr+j, *(ac_fetch *) (data_mem+tshaddr+j));
        }
        //        printf("\n");        
      }

    }
  }
    


  //Search for executable sections (set prog_size_bytes to executable sections only)
  prog_size_bytes = 0;
  lseek(fd, convert_endian(4,ehdr.e_shoff), SEEK_SET);
  for (i=0; i<convert_endian(2,ehdr.e_shnum); i++) {

    int sh_flags;

    if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
      printf("reading ELF section header\n");
      return EXIT_FAILURE;
    }

    sh_flags = convert_endian(4,shdr.sh_flags);

    //if section is executable
    if (sh_flags & SHF_EXECINSTR) {
      int sh_size = convert_endian(4,shdr.sh_size);
      int sh_addr = convert_endian(4,shdr.sh_addr);
      if (prog_size_bytes < sh_size)
        prog_size_bytes = sh_addr + sh_size;
    }

    //next header
    lseek(fd, convert_endian(4,ehdr.e_shoff) + convert_endian(2,ehdr.e_shentsize) * i, SEEK_SET);
  }
  if (prog_size_bytes == 0) prog_size_bytes = data_mem_size;




  //Close file
  close(fd);

  return EXIT_SUCCESS;
}


int accs_ReadHexProgram(char* prog_filename)
{
  FILE *prog;
  unsigned char hex1, hex2;

  //Open application
  if (!(prog = fopen(prog_filename,"r"))) {
    AC_ERROR("Openning application program '%s': %s\n", prog_filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Find program size
  if ((fseek(prog, 0, SEEK_END) != 0) ||
      ((prog_size_bytes = ftell(prog)) == -1) ||
      (fseek(prog, 0, SEEK_SET) != 0) ) {
    AC_ERROR("Can't find file size for %s", prog_filename);
    exit(EXIT_FAILURE);
  }

  //Alocate more realistic size (2 hex digits become 1 byte)
  prog_size_bytes /= 2;
  instr_mem = calloc(prog_size_bytes,1);
  data_mem = instr_mem;

  //Read bytes, count the real program size
  prog_size_bytes = 0;
  while (fscanf(prog, "%c", &hex1) != EOF) {

    //If comment, ignore until end of line
    if (hex1 == '#') {
      while ((fscanf(prog, "%c", &hex1) != EOF) && (hex1 != 10));
    }

    //Not comment, it must be an HEX digit
    else if (! isxdigit(hex1)) {
      //AC_MSG("Reading application program %s (invalid accsim.c)\n", prog_filename);
      free(instr_mem);
      return EXIT_FAILURE;
    }

    //Found one HEX digit, must read another one
    else if ((fscanf(prog, "%c", &hex2) == EOF) ||
             (!isxdigit(hex2))) {
      //AC_MSG("Reading application program %s (invalid accsim.c)\n", prog_filename);
      free(instr_mem);
      return EXIT_FAILURE;
    }
      
    //Convert the 2 HEX digits to byte
    else {
      unsigned char tmp_byte = 0;

      //First hex digit
      hex1 -= '0';
      if (hex1 > 9) hex1 -= 'A' - '0' - 10;
      if (hex1 > 15) hex1 -= 'a' - 'A';
      tmp_byte = hex1 << 4;

      //Second hex digit
      hex2 -= '0';
      if (hex2 > 9) hex2 -= 'A' - '0' - 10;
      if (hex2 > 15) hex2 -= 'a' - 'A';
      tmp_byte |= hex2;

      instr_mem[prog_size_bytes++] = tmp_byte;
    }
  }

  //Copy the program size to the memory size (there is no data in hex format)
  data_mem_size = prog_size_bytes;

  return EXIT_SUCCESS;
}


int accs_ReadObjdumpHex(char* prog_filename) {

  FILE *prog;
  char word[50];
  char line[500];
  char *line_p = line;
  int  chars_read;
  unsigned text_size=0;
  int is_addr, is_text=0, first_addr=1;
  long long data;
  unsigned int  addr=0;
  extern int wordsize;
  extern int ac_match_endian;

  //Open application
  if (!(prog = fopen(prog_filename,"r"))) {
    AC_ERROR("Openning application program '%s': %s\n", prog_filename, strerror(errno));
    exit(EXIT_FAILURE);
  }

  //Alocate the same size of memory as the processor
  prog_size_bytes = accs_FindLoadDevice()->size;
  instr_mem = calloc(prog_size_bytes,1);
  data_mem = instr_mem;

  //Read bytes, count the real program size
  prog_size_bytes = 0;
  while (fgets(line, 500, prog) != NULL) {

    line_p = line;
    is_addr = 1;

    //Processing line
    while(sscanf(line_p, "%50s%n", word, &chars_read) != EOF) {

      line_p += chars_read;

      if (strcmp(word,".text:") == 0){
        is_text = 1;
        continue;
      }
                                                
      if(word[0] == '.' ){
        is_text = 0;
        continue;
      }

      //Processing word
      if( is_addr ){
        addr = strtol(word, NULL, 16);
        is_addr = 0;
        if( is_text && first_addr ){
          first_addr = 0;
          //ac_start_addr = addr;
        }
      }
      else {
                                        
        if(is_text)text_size++;
        data = strtoll(word, NULL, 16);
        switch( wordsize ){
        case 8:
          *((unsigned char *)(instr_mem+(addr))) = (unsigned char)data;
          break;
        case 16:
          *((unsigned short *)(instr_mem+(addr))) = (unsigned short)data;
          break;
        case 32:
          *((unsigned int *)(instr_mem+(addr))) = (unsigned int)data;
          break;
        case 64:
          *((unsigned long long *)(instr_mem+(addr))) = (unsigned long long)data;
          break;
        default:
          AC_ERROR("Wordsize not supported: %d\n", wordsize);
          exit(1);
        }
        addr += wordsize/8;

        //keep the maximum address in prog_size_bytes
        if (prog_size_bytes < addr) prog_size_bytes = addr;

      }
    }
  }
  data_mem_size = prog_size_bytes;
  fclose(prog);
}


////////////////////////////////////////////////////////////////////////////////////
// Create functions ...                                                           //
// These Functions are used to create the compiled simulator files                //
////////////////////////////////////////////////////////////////////////////////////


/*!Create ArchC Compiled Simulation Header File */
/*!Use structures built by the parser.*/
void accs_CreateCompsimHeader()
{
  char filename[256];
  FILE *output;

  sprintf( filename, "%s.H", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Compsim header file.");

  fprintf( output, "#ifndef  _AC_COMPSIM_H\n");
  fprintf( output, "#define  _AC_COMPSIM_H\n");
  fprintf( output, "\n");
  
  //fprintf( output, "#include \"ac_decoder.h\"\n");
  //fprintf( output, "#include \"ac_resources.H\"\n");
  //fprintf( output, "#include \"%s-isa.H\"\n", project_name);
  //fprintf( output, "#include \"%s_isa_behavior.H\"\n", project_name);
  //fprintf( output, "\n");

  /*
  //Defining types for the decoded instruction and compiled simulation table
  fprintf( output, "typedef unsigned *decoded_instr_t;\n");
  fprintf( output, "typedef decoded_instr_t *ac_compsim_table_t;\n");
  fprintf( output, "\n\n");
  */

  //Need a trace?
  fprintf( output,
           "//Generate a trace file if defined\n"
           "//(define also in main.cpp)\n"
           "//#define AC_DEBUG\n"
           "\n"
           );

  //Function prototypes
  fprintf( output, "void Execute(int argc, char *argv[]);\n");
  fprintf( output, "\n");

  fprintf( output, "#endif  //_AC_COMPSIM_H\n");
  fclose( output); 
}


/*!Create ArchC Compiled Simulation Implementation File */
/*!Use structures built by the parser.*/
void accs_CreateCompsimImpl()
{
  extern ac_dec_format *format_list;
  extern ac_dec_instr *instr_list;
  extern int instr_num;
  extern int prog_size_instr;
  extern unsigned char *instr_mem;
  extern char *ACCompsimProg;
  extern int ACABIFlag;

  char filename[256];
  FILE *output;
  int i,j,k, next_instr, rblock;
  int invalid_instr_count = 0;
  int *stat_instr_used = (int *) calloc(instr_num+1, sizeof(int));
  //char *instr_mem_p;
  //char **instr_name;                //!< Instruction name table
  //ac_dec_instr *p_instr_list;       //!< Instruction list

  //CHANGE:
  //if (prog_size_bytes > 5000) prog_size_bytes = 5000;



  //!Create decode table and find number of instructions (allocate more to facilitate delay slots manipulation)
  decode_table = (instr_decode_t **) calloc(prog_size_bytes+16, sizeof(instr_decode_t *));
  // i counts by instructions
  // j counts by bytes
  i=0; next_instr=0;

  for (j=0; j<prog_size_bytes; j++) {

    //If there is an instruction starting in this mem position
    if (next_instr == j) {
      unsigned *fields;
      fields = DECODE(&instr_mem[j]);

      //Perhaps it's an invalid instruction
      if (!fields) {
        //TODO: There is no AC_INSTR_ALIGN, so we act like all instructions have the same size
        //      getting the size of the first instruction
        next_instr += decoder->instructions->size;
        //TODO: change this to be a warning message, like AC_WARN, but with arguments
        if (SHOW_WARNING_INVALID_INSTRUCTION) {
          printf("WARNING: Invalid instruction starting in %#x. Skipping %d bytes (dump:", j, decoder->instructions->size);
          {int d; for(d=0; d<decoder->instructions->size; d++) printf(" %02x", instr_mem[j+d]);}
          printf(").\n");
        }
        invalid_instr_count++;
      }

      //It is a valid instruction
      else {
        //calculate next_instr
        next_instr += GetInstrByID(decoder->instructions, fields[0]) -> size;
        i++;
        //initialize this instr decode vector
        if (!decode_table[j]) decode_table[j] = (instr_decode_t *) calloc(1,sizeof(instr_decode_t));
        decode_table[j]->dec_vector = (unsigned *) malloc(sizeof(unsigned) * decoder->nFields);
        memcpy(decode_table[j]->dec_vector, fields, sizeof(unsigned) * decoder->nFields);
        //verify if it is a control instr and mark the target and the next as leaders
        {
          ac_control_flow *cflow = GetInstrByID(decoder->instructions, fields[0])->cflow;
          if (cflow) {
            eval_input = accs_SubstFields(cflow->target, j);
            if ((eval_parse() != 0) || (eval_result > prog_size_bytes)) {
              //jump register instructions will always report a parse error
              //AC_ERROR("Parsing expression: %s in %#x\n", cflow->target, j);
              //exit(EXIT_FAILURE);
            }
            else {
              //mark the evaluated target as leader instruction (eval_result has the address)
              if (!decode_table[eval_result]) decode_table[eval_result] = (instr_decode_t *) calloc(1,sizeof(instr_decode_t));
              decode_table[eval_result]->is_leader = j;
            }
            //mark also the next as leader
            if (!decode_table[next_instr]) decode_table[next_instr] = (instr_decode_t *) calloc(1,sizeof(instr_decode_t));
            decode_table[next_instr]->is_leader = j;
            //mark also the next+4 as leader   //TODO: depends on delay_slot and instr_size!
            if (!decode_table[next_instr+4]) decode_table[next_instr+4] = (instr_decode_t *) calloc(1,sizeof(instr_decode_t));
            decode_table[next_instr+4]->is_leader = j;
          }
        }
        //update stats
        stat_instr_used[decode_table[j]->dec_vector[0]]++;
      }
    }
  }
  prog_size_instr = i;

  if (invalid_instr_count > 0) { // && (SHOW_WARNING_INVALID_INSTRUCTION)) {
    AC_MSG("WARNING: %d invalid instructions were not decoded.\n", invalid_instr_count);
  }

  //Show instructions used
  if (SHOW_INSTR_USED) {
    AC_MSG("Instructions used:\n");
    for (i=1; (i<instr_num); i++) {
      if (stat_instr_used[i]) {
        printf("     %s #%d\n",
               GetInstrByID(decoder->instructions, i) -> name,
               stat_instr_used[i]);
      }
    }
  }

  AC_MSG("Application size: %u bytes, %u instructions.\n", prog_size_bytes, prog_size_instr);


  //Load more leaders from file if file exists (for opt3 optimization)
  if (PROCESSOR_OPTIMIZATIONS == 3) {
    extern char *ACCompsimProg;
    FILE *leaders;
    char filename[100];
    strcpy(filename, ACCompsimProg);
    strcat(filename, ".leaders");
    if (leaders = fopen(filename, "r")) {
      unsigned leader;
      AC_MSG("Loading leaders from file: %s\n", filename);
      while (fscanf(leaders, "%x", &leader) != EOF) {
        if (leader <= prog_size_bytes) {
          if (!decode_table[leader]) decode_table[leader] = (instr_decode_t *) calloc(1,sizeof(instr_decode_t));
          decode_table[leader]->is_leader = 0xFFFFFFFF;
        }
        else {
          AC_MSG("   the leader %#x is greater then program size (=%#x) or its unnecessary.\n", leader, prog_size_bytes);
        }
      }
    }
    else {
      AC_MSG("File with more leaders does not exist: %s\n", filename);
    }
    
  }



  //Write in separate files, to minimize compilation overhead
  //  select number of Regions per file with command-line option "-bs"
  for (rblock=0; rblock <= (((prog_size_bytes-1) >> REGION_SIZE) / REGION_BLOCK_SIZE); rblock++) {

    // Open file
    if (rblock == 0) sprintf( filename, "%s.cpp", project_name);
    else             sprintf( filename, "%s-block%d.cpp", project_name, rblock);

    if ( !(output = fopen( filename, "w"))){
      perror("ArchC could not open output file");
      exit(1);
    }


    //!Write file header
    print_comment( output, "ArchC Compsim implementation file.");

    if (rblock == 0) {
      fprintf(output, "//Input program: %s\n\n", ACCompsimProg);
      fprintf( output, "#include \"%s.H\"\n", project_name);
    }

    fprintf( output, "#include \"%s-isa.H\"\n", project_name);

    if (rblock == 0) {

      if (ACABIFlag) {
        fprintf( output, "#include \"%s_syscall.H\"\n", project_name);
      }

      fprintf( output, "#include \"ac_prog_regions.H\"\n");
    }

    fprintf( output, "\n");


    // Inlines for command-line option "-i"
    fprintf( output, "#ifdef AC_INLINE\n");
    fprintf( output, "\n");
    fprintf( output, "//ISA instructions are compiled together for inline to work\n");
    fprintf( output, "#include \"%s-isa.cpp\"\n", project_name);
    fprintf( output, "\n");
    if (rblock == 0) {
      if (ACABIFlag) {
        fprintf( output, "//Syscalls are compiled together for inline to work\n");
        fprintf( output, "#include \"%s_syscall.cpp\"\n", project_name);
        fprintf( output, "\n");
      }
    }
    fprintf( output, "#endif // defined AC_INLINE\n");
    fprintf( output, "\n");


    // Trace file for command-line option "-g"
    fprintf( output,
             "//!Trace file\n"
             "#ifdef AC_DEBUG\n"
             "#include <iostream>\n"
             "extern std::ofstream trace_file;\n"
             "#define PRINT_TRACE trace_file << std::hex << ((int)ac_pc) << \"\\n\"\n"
             "#else \n"
             "#define PRINT_TRACE /* nothing */\n"
             "#endif\n"
             "\n");


    // Write other variables
    if (rblock == 0) {
      if (COUNT_SYSCALLS)  COUNT_SYSCALLS_Init(output);
      if (ACABIFlag)       fprintf(output, "%s_syscall model_syscall;\n\n", project_name);
    }


    //Declarations for PROCESSOR_OPTIMIZATIONS
    if (PROCESSOR_OPTIMIZATIONS > 0) {
      fprintf(output, "//Used for optimizations\n");
      fprintf(output, "unsigned tmp_pc;\n\n");
    }


    // @@@@@@@@@@@@@@@@@ kernel of compiled simulation @@@@@@@@@@@@@@@@@@@@@@@

    for (i=rblock*REGION_BLOCK_SIZE;
         ((i < (rblock+1)*REGION_BLOCK_SIZE) && (i <= ((prog_size_bytes-1) >> REGION_SIZE)));
         i++) {
      int end_region = (prog_size_bytes < ((i+1) << REGION_SIZE)) ? prog_size_bytes : ((i+1) << REGION_SIZE);
    
      // Region function start
      fprintf(output, "void Region%d() {\n", i);
      fprintf(output,
              "\n"
              "  while (1) {\n"
              "    switch((int)ac_pc) {\n"
              "\n"
              );


      // KERNEL: emit instructions
      for (j = (i << REGION_SIZE); j < end_region; j++) {

        if ((ACABIFlag) && (j==64)) {  // Special addresses for ArchC system calls
          j = accs_EmitSyscalls(output, j);
        }

        if ((decode_table[j]) && (decode_table[j]->dec_vector)) {
          accs_EmitInstr(output, j);
        }
      }


      // Write invalid PC section and end Region function
      fprintf( output,
               "      //certify to change region on fall-through\n"
               "      ac_pc = %#x;\n"
               "      break;\n"
               "\n"
               "    default:\n"
               , ((i+1) << REGION_SIZE));
      if (i == EXIT_ADDRESS>>REGION_SIZE) fprintf( output, "      if (ac_pc == %d) return;\n", EXIT_ADDRESS);
      fprintf( output,
               "      if ((ac_pc >= %d) && (ac_pc < %d)) {\n"
               "        AC_ERROR(\"ac_pc=0x\" << hex << int(ac_pc) << \" points to an non-decoded memory location.\" << endl);\n"
               "        ac_stop(EXIT_FAILURE);\n"
               "      }\n"
               "      return;\n"
               "    }\n"
               "  }\n"
               "}\n"
               "\n"
               "\n"
               , (i << REGION_SIZE), ((i+1) << REGION_SIZE));
    }


    // Write Execute function, only in the first file
    if (rblock == 0) {

      fprintf(output, "void Execute(int argc, char *argv[]) {\n\n");
      if (ACABIFlag) {fprintf(output, "  model_syscall.set_prog_args(argc, argv);\n\n");}
      fprintf(output,
              "  extern int ac_stop_flag;\n"
              "  while (!ac_stop_flag) {\n"
              "    switch(ac_pc >> %d) {\n"
              "\n"
              , REGION_SIZE);
      for (j=0; j <= ((prog_size_bytes-1) >> REGION_SIZE); j++) {
        fprintf(output,
                "    case %d:\n"
                "      Region%d();\n"
                "      break;\n"
                "\n"
                //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
                , j, j);
      }
      fprintf(output,
              "    default:\n"
              "      AC_ERROR(\"ac_pc=0x\" << hex << int(ac_pc) << \" is beyond program memory.\" << std::endl);\n"
              "      ac_stop(EXIT_FAILURE);\n"
              "      break;\n"
              "    }\n"
              "  }\n"
              );

      if (COUNT_SYSCALLS) {
        COUNT_SYSCALLS_EmitPrintStat(output);
      }

      fprintf(output,
              "}\n"
              "\n"
              "\n"
              );
    }


    // Close file
    fclose( output);

  }


  // Free memory
  for (j=0; j<prog_size_bytes; j++) {free(decode_table[j]);}
  free(decode_table);
  free(stat_instr_used);
}


/*!Create ArchC Compiled Simulation Header for Region prototypes */
void accs_CreateCompsimHeaderRegions()
{
  int i;
  FILE *output;

  if ( !(output = fopen("ac_prog_regions.H", "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Compsim header for region prototypes.");

  for (i=0; i <= ((prog_size_bytes-1) >> REGION_SIZE); i++) {
    fprintf(output, "void Region%d();\n", i);
  }

  fclose( output); 
}


/*!Create ArchC Storage Header File */
/*!Use structures built by the parser.*/
void fast_CreateStorageHeader()
{
  extern int ac_tgt_endian;
  FILE *output;

  output = fopen("ac_storage.H", "w");

  print_comment( output, "ArchC Storage header file.");

  fprintf(output,
          "#ifndef  _AC_STORAGE_H\n"
          "#define  _AC_STORAGE_H\n"
          "\n"
          "#include <stdio.h>\n"
          "#include <iostream>\n"
          "#include <fstream>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "//#include \"ac_config.H\"\n"
          "#include \"ac_parms.H\"\n"
          "#include \"archc.H\"\n"
          "\n"
          "//#define AC_STORAGE_VERIFY\n"
          "\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "#include \"archc.H\"\n"
          "#endif\n"
          "\n");

  if (ACStatsFlag) {
    fprintf(output, "#include \"ac_stats.H\"\n");
    fprintf(output, "extern ac_stats ac_sim_stats;\n\n");
  }
    
  if (ACDelayFlag) {
    fprintf(output,
            "// DELAYED QUEUE\n"
            "\n"
            "#define MAX_DELAY 3\n"
            "\n"
            "struct delayed_word_t {\n"
            "  ac_Uword* ref;\n"
            "  ac_Uword  val;\n"
            "};\n"
            "\n"
            "extern delayed_word_t delayed_queue_word[MAX_DELAY+1];\n"
            "\n"
            "\n"
            "struct delayed_byte_t {\n"
            "  ac_Ubyte* ref;\n"
            "  ac_Ubyte  val;\n"
            "};\n"
            "\n"
            "extern delayed_byte_t delayed_queue_byte[MAX_DELAY+1];\n"
            "\n"
            "\n"
            "\n"
            "// DELAY CLASS\n"
            "\n"
            "class delay {\n"
            "public:\n"
            "  ac_Uword val;\n"
            "  int    d;\n"
            "  static int def;\n"
            "  static void do_assignment();\n"
            "\n"
            "  explicit delay(ac_Uword newval, int newd=-1) : val(newval), d(newd) {\n"
            "#ifdef AC_STORAGE_VERIFY\n"
            "    if ((newd < 0) || (newd > MAX_DELAY)) d=def;\n"
            "#endif\n"
            "  }\n"
            "\n"
            "  operator ac_Uword const () {return val;}\n"
            "};\n"
            "\n");
  } /* end if (ACDelayFlag) */

  fprintf(output,
          "\n"
          "\n"
          "\n"
          "// STORAGE COMPONENTS CLASS\n"
          "\n"
          "class reg_word_t {\n"
          "  ac_Uword val;\n"
          "public:\n"
          "  reg_word_t() {}\n"
          "  reg_word_t(ac_Uword i) : val(i) {}\n"
          "  operator ac_Uword&()  {return (ac_Uword&) val;}\n"
          "  template <class T> operator T() {return val;}\n"
          "\n");

  if (ACDelayFlag) {
    fprintf(output,
            "  delay operator = (delay d) {\n"
            "    delayed_queue_word[d.d].ref = &val;\n"
            "    delayed_queue_word[d.d].val = d.val;\n"
            "    return d;\n"
            "  }\n"
            "\n");
  } /* end if (ACDelayFlag) */

  fprintf(output,
          "  template <class T>\n"
          "  ac_Uword operator = (T newval) {\n"
          "    return val = newval;\n"
          "  }\n"
          "\n"
          "  //ac_Uword read(unsigned addr) {return *(&val+addr)}\n"
          "  //void write(unsigned addr, unsigned datum) {*(&val+addr) = datum;}\n"
          "};\n"
          "\n"
          "\n"
          "class reg_byte_t {\n"
          "  ac_Ubyte val;\n"
          "public:\n"
          "  reg_byte_t() {}\n"
          "  reg_byte_t(ac_Ubyte i) : val(i) {}\n"
          "  operator ac_Ubyte&()  {return (ac_Ubyte&) val;}\n"
          "\n");

  if (ACDelayFlag) {
    fprintf(output,
            "  delay operator = (delay d) {\n"
            "    delayed_queue_byte[d.d].ref = &val;\n"
            "    delayed_queue_byte[d.d].val = d.val;\n"
            "    return d;\n"
            "  }\n"
            "\n");
  } /* end if (ACDelayFlag) */

  fprintf(output,
          "};\n"
          "\n"
          "\n"
          /* start reg_uint */
          "class reg_uint {\n"
          "  unsigned val;\n"
          "public:\n"
          "  reg_uint() {}\n"
          "  reg_uint(unsigned i) : val(i) {}\n"
          "  operator unsigned&()  {return (unsigned&) val;}\n"
          "  template <class T> operator T() {return val;}\n"
          "\n");

  if (ACDelayFlag) {
    fprintf(output,
            "  delay operator = (delay d) {\n"
            "    delayed_queue_word[d.d].ref = &val;\n"
            "    delayed_queue_word[d.d].val = d.val;\n"
            "    return d;\n"
            "  }\n"
            "\n");
  } /* end if (ACDelayFlag) */

  fprintf(output,
          "  template <class T>\n"
          "  unsigned operator = (T newval) {\n"
          "    return val = newval;\n"
          "  }\n"
          "\n"
          "  unsigned read() {return val;}\n"
          "  void write(unsigned datum) {val=datum;}\n"
          "};\n"
          "\n"
          "\n"
          /* end reg_uint */

          "\n"
          "\n"
          "\n"
          "// STORAGE CLASS\n"
          "\n"
          "template <class elem_t, class access_t>\n"
          "class ac_storage_tmpl {\n"
          "\n"
          "protected:\n"
          "  elem_t *Data;\n"
          "  char *name;\n"
          "  unsigned int start;\n"
          "  unsigned int size;\n"
          "\n"
          "public:\n"
          "\n"
          "  access_t read() const {return read(0);}\n"
          "  void write(access_t datum) {return write(0,datum);}\n"
          "\n"
          "  unsigned int GetStart() {return start;}\n"
          "\n"
          "  elem_t *raw_data(unsigned address) const\n"
          "  {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    if (address-start < size)\n"
          "#endif\n"
          "      return &Data[address-start];\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    else {\n"
          "      AC_RUN_ERROR(\"Storage %%s: raw_data address not in memory: 0x%%x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "#endif\n"
          "  }\n"
          "\n"
          "  access_t read(unsigned address) const {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    int unalign = (sizeof(access_t) != sizeof(elem_t)) ? (sizeof(access_t)-1) : 0;\n"
          "    if (address & unalign) {\n"
          "      AC_RUN_ERROR(\"Storage %%s: read word address unaligned: %%#x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "    if (address < size)\n"
          "#endif\n"
          "      return *(access_t*) &Data[address-start];\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    else {\n"
          "      AC_RUN_ERROR(\"Storage %%s: read address not in memory: 0x%%x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "#endif\n"
          "  }\n"
          "\n"
          "  access_t read_byte(unsigned address) const {\n"
          "    int unalign = sizeof(ac_Uword)-1;\n"
          "    access_t aligned = read(address & ~unalign);\n"
          "    int shift = %s;\n"
          "    return (aligned & (0xFF << shift)) >> shift;\n"
          "  }\n"
          "\n"
          "  access_t read_half(unsigned address) const {\n"
          "    int unalign = sizeof(ac_Uword)-1;\n"
          "    access_t aligned = read(address & ~unalign);\n"
          "    int shift = %s;\n"
          "    return (aligned & (0xFFFF << shift)) >> shift;\n"
          "  }\n"
          "\n"
          "  void write(unsigned address, access_t datum) {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    int unalign = (sizeof(access_t) != sizeof(elem_t)) ? (sizeof(access_t)-1) : 0;\n"
          "    if (address & unalign) {\n"
          "      AC_RUN_ERROR(\"Storage %%s: write word address unaligned: %%#x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "    if (address < size)\n"
          "#endif\n"
          "      (access_t&) Data[address] = datum;\n"
          "#ifdef AC_STORAGE_VERIFY\n"
          "    else {\n"
          "      AC_RUN_ERROR(\"Storage %%s: write address not in memory: 0x%%x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "#endif\n"
          "  }\n"
          "\n"
          "  void write_byte(unsigned address, unsigned char datum) {\n"
          "    int unalign = sizeof(ac_Uword)-1;\n"
          "    access_t aligned = *(access_t*) &Data[(address & ~unalign)-start];\n"
          "    int shift = %s;\n"
          "    write(address & ~unalign, (aligned & ~(0xFF << shift)) | (datum << shift) );\n"
          "  }\n"
          "\n"
          "  void write_half(unsigned address, unsigned short datum) {\n"
          "    int unalign = sizeof(ac_Uword)-1;\n"
          "    access_t aligned = *(access_t*) &Data[(address & ~unalign)-start];\n"
          "    int shift = %s;\n"
          "    write(address & ~unalign, (aligned & ~(0xFFFF << shift)) | (datum << shift) );\n"
          "  }\n"
          "\n"
          "  void load( char* file );\n"
          "\n"
          "  void load_array( const unsigned char* d, const unsigned s )\n"
          "  {\n"
          "    if (size < s) {\n"
          "      fprintf(stderr, \"Storage %%s: trying to load an array bigger then storage size.\\n\", name);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "    memcpy(Data, d, s);\n"
          "  }\n"
          "\n"
          "  void load_binary( const char* progmem )\n"
          "  {\n"
          "    FILE *file;\n"
          "    unsigned long fileLen;\n"
          "\n"
          "    //Open file\n"
          "    file = fopen(progmem, \"rb\");\n"
          "    if (!file) {\n"
          "      AC_ERROR(\"Storage \" << name << \": Unable to open binary memory copy from file \" << progmem);\n"
          "      return;\n"
          "    }\n"
          "\n"
          "    //Get file length\n"
          "    fseek(file, 0, SEEK_END);\n"
          "    fileLen=ftell(file);\n"
          "    fseek(file, 0, SEEK_SET);\n"
          "\n"
          "    //Test size\n"
          "    if (size < fileLen) {\n"
          "      AC_ERROR(\"Storage \" << name << \": trying to load a file (\" << fileLen << \" bytes) bigger then storage size (\" << size << \" bytes)\");\n"
          "      fclose(file);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "\n"
          "    //Read file contents into buffer\n"
          "    fread(Data, fileLen, 1, file);\n"
          "    fclose(file);\n"
          "  }\n"
          "\n"
          "  //!Constructors.\n"
          "  ac_storage_tmpl( char *n, unsigned _size=0, unsigned _start=0, unsigned char *contents=0, unsigned c_size=0 ){\n"
          "    name = n;\n"
          "    start = _start;\n"
          "    size = (_size > c_size)? _size : c_size;\n"
          "    Data = new elem_t [size];\n"
          //"    memset(Data, 0, size);\n"
          "    if (contents) memcpy(Data, contents, c_size);\n"
          "    //cout << n << \" initialized.\" << endl;\n"
          "    if (start != 0) {\n"
          "      fprintf(stderr, \"Storage %%s: start value not igual to zero not supported\\n\", name);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "  }\n"
          "\n"
          "  //!Destructor\n"
          "  ~ac_storage_tmpl() {\n"
          "    delete[] Data;\n"
          "  }\n"
          "\n"
          "  access_t operator[] (unsigned address) {\n"
          "    if (address-start < size)\n"
          "      return *(access_t*) &Data[address-start];\n"
          "    else {\n"
          "      fprintf(stderr, \"Storage %%s: access address not in memory: 0x%%x\\n\", name, address);\n"
          "      exit(EXIT_FAILURE);\n"
          "    }\n"
          "  }\n"
          "\n"
          "};\n"
          "\n"

          "template <std::size_t SIZE>\n"
          "class ac_regbank {\n"
          "\n"
          "protected:\n"
          "  ac_Uword Data[SIZE];\n"
          "  char *name;\n"
          "  unsigned int size;\n"
          "\n"
          "public:\n"
          "\n"
          "  ac_Uword read() const {return read(0);}\n"
          "  void write(ac_Uword datum) {return write(0,datum);}\n"
          "\n"
          "  ac_Uword read(unsigned address) const {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "      return Data[address];\n"
          "  }\n"
          "\n"
          "  void write(unsigned address, ac_Uword datum) {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "      Data[address] = datum;\n"
          "  }\n"
          "\n"
          "  //!Constructors.\n"
          "  ac_regbank( char *n, unsigned _size=0 ) {\n"
          "    name = n;\n"
          "    size = _size;\n"
          //"    Data = new ac_Uword [size];\n"
          //"    memset(Data, 0, size);\n"
          "  }\n"
          "\n"
          "  //!Destructor\n"
          "  ~ac_regbank() {\n"
          //"    delete[] Data;\n"
          "  }\n"
          "\n"
          "  const ac_Uword& operator[] (unsigned address) const {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "      return Data[address];\n"
          "  }\n"
          "\n"
          "  ac_Uword& operator[] (unsigned address) {\n"
          "#ifdef AC_STATS\n"
          "    ac_sim_stats.add_access(name);\n"
          "#endif\n"
          "      return Data[address];\n"
          "  }\n"
          "\n"
          "};\n"
          "\n"
          "\n"
          "\n"
          "\n"
          "//!Types for storages\n"
          "typedef ac_storage_tmpl<ac_byte, ac_Uword> ac_storage;\n"
          //"typedef ac_storage_Uword_Uword ac_regbank;\n"
          "\n"
          "\n"

          //"//!Types for storages\n"
          //"typedef ac_storage_tmpl<ac_byte, ac_Uword> ac_storage;\n"
          //"typedef ac_storage_tmpl<ac_Uword, ac_Uword> ac_regbank;\n"
          //"\n"
          //"\n"
          "#endif\n"
          //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
          , (ac_tgt_endian)? "(unalign - (address & unalign))*8" : "(address & unalign)*8"
          , (ac_tgt_endian)? "(2 - (address & unalign))*8" : "(address & unalign)*8"
          , (ac_tgt_endian)? "(unalign - (address & unalign))*8" : "(address & unalign)*8"
          , (ac_tgt_endian)? "(2 - (address & unalign))*8" : "(address & unalign)*8"
          );

  fclose( output); 
}


/*!Create ArchC Storage Implementation File */
/*!Use structures built by the parser.*/
void fast_CreateStorageImpl()
{
  FILE *output;
  output = fopen("ac_storage.cpp", "w");

  print_comment( output, "ArchC Storage implementation file.");

  fprintf(output,
          "#include <errno.h>\n"
          "#include \"ac_storage.H\"\n"
          "#include \"archc.H\"\n"
          "#include \"ac_parms.H\"\n"
          "\n"
          "\n");

  if (ACDelayFlag) {
    fprintf(output,
            "// DELAYED QUEUE\n"
            "delayed_word_t delayed_queue_word[MAX_DELAY+1];\n"
            "delayed_byte_t delayed_queue_byte[MAX_DELAY+1];\n"
            "\n"
            "\n"
            "int delay::def=1;\n"
            "\n"
            "\n"
            "void delay::do_assignment() {\n"
            "  if (delayed_queue_word[0].ref)  *(delayed_queue_word[0].ref) = delayed_queue_word[0].val;\n"
            "\n"
            "  for (int i=0; i<MAX_DELAY; i++) delayed_queue_word[i] = delayed_queue_word[i+1];\n"
            "  delayed_queue_word[MAX_DELAY].ref = 0;\n"
            "\n"
            "  if (delayed_queue_byte[0].ref)  *(delayed_queue_byte[0].ref) = delayed_queue_byte[0].val;\n"
            "\n"
            "  for (int i=0; i<MAX_DELAY; i++) delayed_queue_byte[i] = delayed_queue_byte[i+1];\n"
            "  delayed_queue_byte[MAX_DELAY].ref = 0;\n"
            "};\n"
            "\n"
            );
  }

  fprintf(output,
"//!Method to load  content from a file.\n"
"template <class elem_t, class access_t>\n"
"void ac_storage_tmpl<elem_t, access_t>::load( char* file ){\n"
"\n"
"  FILE *prog;\n"
"  char word[50];\n"
"  char line[500];\n"
"  char *line_p = line;\n"
"  int  chars_read;\n"
"  unsigned text_size=0;\n"
"  int is_addr, is_text=0, first_addr=1;\n"
"  long long data;\n"
"  unsigned int  addr=0;\n"
"\n"
"  //Open application\n"
"  if (!(prog = fopen(file,\"r\"))) {\n"
"    AC_ERROR(\"Openning application program '\" << file << \"': \" << strerror(errno));\n"
"    exit(EXIT_FAILURE);\n"
"  }\n"
"\n"
"  //Read bytes\n"
"  while (fgets(line, 500, prog) != NULL) {\n"
"\n"
"    line_p = line;\n"
"    is_addr = 1;\n"
"\n"
"    //Processing line\n"
"    while(sscanf(line_p, \"%%50s%%n\", word, &chars_read) != EOF) {\n"
"\n"
"      line_p += chars_read;\n"
"\n"
"      if (strcmp(word,\".text:\") == 0){\n"
"        is_text = 1;\n"
"        continue;\n"
"      }\n"
"                                                \n"
"      if(word[0] == '.' ){\n"
"        is_text = 0;\n"
"        continue;\n"
"      }\n"
"\n"
"      //Processing word\n"
"      if( is_addr ){\n"
"        addr = strtol(word, NULL, 16);\n"
"        is_addr = 0;\n"
"        if( is_text && first_addr ){\n"
"          first_addr = 0;\n"
"          //ac_start_addr = addr;\n"
"        }\n"
"      }\n"
"      else {\n"
"                                        \n"
"        if(is_text)text_size++;\n"
"        data = strtoll(word, NULL, 16);\n"
"        // TODO: correct endiannes or no?\n"
"        if (1) {\n"
"          char *buffer = (char *) &data;\n"
"          Data[addr+0] = buffer[0];\n"
"          Data[addr+1] = buffer[1];\n"
"          Data[addr+2] = buffer[2];\n"
"          Data[addr+3] = buffer[3];\n"
"        }\n"
"        else {\n"
"          char *buffer = (char *) &data;\n"
"          Data[addr+0] = buffer[3];\n"
"          Data[addr+1] = buffer[2];\n"
"          Data[addr+2] = buffer[1];\n"
"          Data[addr+3] = buffer[0];\n"
"        }\n"
"        addr += AC_WORDSIZE/8;\n"
"        if (addr > size) {\n"
"          AC_ERROR(\"Loaded file \" << file << \"is bigger then available memory.\");\n"
"          exit(1);\n"
"        }\n"
"      }\n"
"    }\n"
"  }\n"
"  fclose(prog);\n"
"}\n"
"\n"
          );

  fclose( output); 
}


/*!Create ArchC Resources Header File */
/*!Use structures built by the parser.*/
void fast_CreateResourcesHeader()
{
  extern ac_sto_list *storage_list;
  extern int HaveMemHier, HaveFormattedRegs;
  ac_sto_list *pstorage;

  FILE *output;
  output = fopen("ac_resources.H", "w");

  print_comment( output, "ArchC Resources header file.");

  fprintf( output, "#ifndef  _AC_RESOURCES_H\n");
  fprintf( output, "#define  _AC_RESOURCES_H\n\n");

  fprintf( output, "#include  \"ac_parms.H\"\n\n");
  fprintf( output, "#include  \"ac_storage.H\"\n");
  //fprintf( output, "#include  \"ac_reg.H\"\n\n");

  if( HaveMemHier ){
    fprintf( output, "#include  \"ac_mem.H\"\n");
    fprintf( output, "#include  \"ac_cache.H\"\n");
  }

  if( HaveFormattedRegs )
    fprintf( output, "#include  \"ac_fmt_regs.H\"\n");
  fprintf( output, " \n");


  COMMENT(INDENT0,"Namespace for compatibility with interpreted ArchC.","\n");
  fprintf( output, "namespace ac_resources {\n\n");

  /* Declaring storage devices */
  COMMENT(INDENT[1],"Storage Devices.");
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

    switch( pstorage->type ){

    case REG:

      //Formatted registers have a special class.
      if( pstorage->format != NULL ){
        fprintf( output, "%sextern ac_%s %s;\n", INDENT[1], pstorage->name, pstorage->name);
      }
      else{
        fprintf( output, "%sextern ac_regbank<1> %s;\n", INDENT[1], pstorage->name);      
      }
      break;
                        
    case REGBANK:
      fprintf( output, "%sextern ac_regbank<%u> %s;\n", INDENT[1], pstorage->size, pstorage->name);
      break;

    case CACHE:
    case ICACHE:
    case DCACHE:

      if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%sextern ac_storage %s;\n", INDENT[1], pstorage->name);
      }
      else{
        //It is an ac_cache object.
        fprintf( output, "%sextern ac_cache %s;\n", INDENT[1], pstorage->name);
      }

      break;

    case MEM:

      if( !HaveMemHier ) { //It is a generic mem. Just emit a base container object.
        fprintf( output, "%sextern ac_storage %s;\n", INDENT[1], pstorage->name);
      }
      else{
        //It is an ac_mem object.
        fprintf( output, "%sextern ac_mem %s;\n", INDENT[1], pstorage->name);
      }

      break;

      
    default:
      fprintf( output, "%sextern ac_storage %s;\n", INDENT[1], pstorage->name);      
      break;
    }
  }

  fprintf( output, "\n");

  COMMENT(INDENT[1],"Control Variables.","\n");
  fprintf( output, "  extern reg_uint ac_pc;\n");
  //fprintf( output, "  extern reg_word_t ac_branch;\n");
  fprintf( output, "  extern unsigned long long ac_instr_counter;\n");
  fprintf( output, "  extern bool ac_wait_sig;\n");
  fprintf( output, "  extern bool ac_tgt_endian;\n");
  fprintf( output, "  extern unsigned ac_start_addr;\n\n");

  fprintf( output, "  void ac_wait();\n");
  fprintf( output, "  void ac_release();\n");

  /* Close namespace ac_resources */
  fprintf( output, "}\n\n");

  COMMENT(INDENT[0],"Global aliases for resources.");
  fprintf( output, "using namespace ac_resources;\n\n");

  fprintf( output, "#endif  //_AC_RESOURCES_H\n");

  fclose( output);
}


/*!Create ArchC Resources Implementation File */
/*!Use structures built by the parser.*/
void fast_CreateResourcesImpl()
{
  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;
  extern int HaveMemHier;
  extern int ac_tgt_endian;

  int i;

  FILE *output;
  output = fopen("ac_resources.cpp", "w");

  print_comment( output, "ArchC Resources implementation file.");

  fprintf( output, "#include  \"ac_resources.H\"\n\n");

  COMMENT(INDENT0,"Namespace for compatibility with interpreted ArchC.","\n");
  fprintf( output, "namespace ac_resources {\n\n");

  COMMENT(INDENT[1],"Storage Devices.");
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

    switch( pstorage->type ){

    case REG:

      //Formatted registers have a special class.
      if( pstorage->format != NULL ){
        fprintf( output, "%sac_%s %s(\"%s\");\n", INDENT[1], pstorage->name, pstorage->name, pstorage->name);
      }
      else{
        fprintf( output, "%sac_regbank<1> %s(\"%s\", 1);\n", INDENT[1], pstorage->name, pstorage->name);
      }
      break;
                        
    case REGBANK:
      fprintf( output, "%sac_regbank<%u> %s(\"%s\", %d);\n", INDENT[1], pstorage->size, pstorage->name, pstorage->name, pstorage->size);
      break;

    case CACHE:
    case ICACHE:
    case DCACHE:

      if( !pstorage->parms ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%sac_storage %s(\"%s\", %d);\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      }
      else{
        //It is an ac_cache object.
        EmitCacheDeclaration(output, pstorage, 1);
      }
      break;

    case MEM:

      if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%sac_storage %s(\"%s\", %d);\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      }
      else{
        //It is an ac_mem object.
        fprintf( output, "%sac_mem %s(\"%s\", %d);\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      }
      break;

    default:
      fprintf( output, "%sac_storage %s(\"%s\", %d);\n", INDENT[1], pstorage->name, pstorage->name, pstorage->size);
      break;
    }
  }

  /*
    COMMENT(INDENT0,"The global and only RESOURCES object.","\n");
    fprintf( output, "ac_resources RESOURCES;\n\n");

    //!Declaring Constructor.
    COMMENT(INDENT0,"Constructor.","\n");
    fprintf( output, "  ac_resources::ac_resources(): ");
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next)
    fprintf( output, "%s(\"%1$s\",%2$d), ", pstorage->name, pstorage->size);
    fprintf(output , "IM(\"IM\",%d) {\n\n", prog_size_instr);
  */

  fprintf( output, "\n");

  COMMENT(INDENT[1],"Control Variables","\n");
  fprintf( output, "  reg_uint ac_pc = %d;\n", prog_entry_point);
  //fprintf( output, "  reg_word_t ac_branch = 0;\n");
  fprintf( output, "  unsigned long long ac_instr_counter = 0;\n");
  fprintf( output, "  bool ac_wait_sig;\n");
  fprintf( output, "  bool ac_tgt_endian = %d;\n", ac_tgt_endian);
  fprintf( output, "  unsigned ac_start_addr = 0;\n");

  fprintf( output, "\n");
  fprintf( output, "  void ac_wait(){\n");
  fprintf( output, "    ac_wait_sig = 1;\n");
  fprintf( output, "  };\n");
  fprintf( output, "\n");
  fprintf( output, "  void ac_release(){\n");
  fprintf( output, "    ac_wait_sig = 0;\n");
  fprintf( output, "  };\n");
  fprintf( output, "\n");
  fprintf( output, "}\n\n");

  /*
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next)
    fprintf( output, "  for(int i=0; i<%d; i++) %s[i]=0;\n", pstorage->size, pstorage->name);
    fprintf( output, "  for(int i=0; i<%d; i++) IM[i]=0;\n\n", prog_size_instr);
  */

  /* There is no need to load the program into memory!  TODO: is there a need for IM?
     for (i=0; i<prog_size_instr; i++)
     fprintf( output, "  IM[%d] = %u;\n", i, prog_instr[i]);
     fprintf( output, "\n");
  */
    
  //fprintf( output, "};\n\n");

  fclose( output); 
}


/*!Create ArchC Program Memory Header File */
void accs_CreateProgramMemoryHeader()
{
  int i;
  FILE *output;
  char filename[] = "ac_progmem.H";
  extern char *ACCompsimProg;

  if ( !(output = fopen( filename, "wb"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  fprintf( output, "#ifdef __CYGWIN__\n");
  fprintf( output, "#define INCBIN(symname, filename) INCBIN2(symname, _##symname, filename)\n");
  fprintf( output, "#else\n");
  fprintf( output, "#define INCBIN(symname, filename) INCBIN2(symname, symname, filename)\n");
  fprintf( output, "#endif\n");
  fprintf( output, "\n");
  fprintf( output, "\n");
  fprintf( output, "#define INCBIN2(symname, binname, filename) \\\n");
  fprintf( output, "   __asm__ (\".section .data ; .align 2; .globl \" #binname); \\\n");
  fprintf( output, "   __asm__ (#binname \":\\n.incbin \\\"\" filename \"\\\"\"); \\\n");
  fprintf( output, "   extern unsigned char symname[];\n");
  fprintf( output, "\n");
  fprintf( output, "//Input program: %s\n", ACCompsimProg);
  fprintf( output, "INCBIN(mem_dump, \"ac_progmem.bin\");\n");

  fclose( output); 
}


//!Create ArchC Program Memory array for compiled simulation
void accs_CreateProgramMemoryImpl()
{
  /* Create binary file */
  int i;
  FILE *output;
  char filename[] = "ac_progmem.bin";

  if ( !(output = fopen( filename, "wb"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  fwrite(data_mem, ac_heap_ptr, 1, output);

  fclose( output);
}


/*!Create ArchC Storage Implementation File */
/*!Use structures built by the parser.*/
void accs_CreateMain()
{
  extern char *arch_filename;
  extern char *ACCompsimProg;
  extern char *ACVersion;
  extern char ACOptions[500];
  int virtual_RAMSIZE=0;
  char filename[] = "main.cpp.tmpl";
  char description[256];
  FILE  *output;

  sprintf( description, "This is the main file for the %s ArchC model", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, description);

  fprintf(output,
          "#include \"archc.H\"\n"
          //"#include \"archc.cpp\"\n"
          "#include \"ac_resources.H\"\n"
          "#include \"%s-isa.H\"\n"
          "#include \"ac_progmem.H\"\n"
          "\n"
          "\n"
          , project_name);

  fprintf( output, "const char *project_name=\"%s\";\n", project_name);
  fprintf( output, "const char *project_file=\"%s\";\n", arch_filename);
  fprintf( output, "const char *archc_version=\"%s\";\n", ACVersion);
  fprintf( output, "const char *archc_options=\"%s\";\n", ACOptions);
  fprintf( output, "\n");

  if (ACStatsFlag) {
    fprintf(output, "#include \"ac_stats.H\"\n");
    fprintf(output, "ac_stats ac_sim_stats;\n\n");
  }

  fprintf(output,
          "int main(int ac, char *av[])\n"
          "{\n"
          "  //ac_resources %s;\n"
          //"#include \"ac_progmem.cpp\"\n"
          //"  extern unsigned char mem[%u];\n"
          "  extern char *appfilename;\n"
          "  appfilename=\"%s\";\n"
          "\n"
          "  ac_heap_ptr = 0x%x;\n"
          "  ac_start_addr = 0x%x;\n"
          "\n"
          "  %s.load_array(mem_dump, 0x%x);\n"
          "\n"
          "#ifdef AC_DEBUG\n"
          "  ac_trace(\"%s.trace\");\n"
          "#endif\n"
          "\n"
          "  ac_init();\n"
          "\n"
          "  ac_start();\n"
          "\n"
          "  std::cerr << std::endl;\n"
          "  PrintStat();\n"
          "  std::cerr << std::endl;\n"
          "\n"
          //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
          , project_name, ACCompsimProg, ac_heap_ptr, ac_start_addr, (accs_FindLoadDevice())->name
          , ac_heap_ptr, project_name);

  if (ACStatsFlag)
    fprintf(output,
            "#ifdef AC_STATS\n"
            "  ac_sim_stats.time = 0;\n"
            "  ac_sim_stats.instr_executed = ac_instr_counter - ac_sim_stats.syscall_executed;\n"
            "  ac_sim_stats.print();\n"
            "#endif \n"
            "\n");

  fprintf(output,
          "#ifdef AC_DEBUG\n"
          "  ac_close_trace();\n"
          "#endif\n"
          "\n"
          "  return ac_exit_status;\n"
          "}\n"
          );

  fclose( output); 
}


/*!Create ArchC ISA Header File */
/*!Use structures built by the parser.*/
void accs_CreateISAHeader()
{
  extern char *project_name;
  extern ac_decoder_full *decoder;
  ac_dec_field *pfield;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;

  FILE *output;
  char filename[50];
  
  snprintf(filename, 50, "%s-isa.H", project_name);
  output = fopen(filename, "w");

  print_comment( output, "ISA header file.");

  fprintf(output,
          "#ifndef _ISA_H\n"
          "#define _ISA_H\n"
          "\n"
          "#include \"ac_resources.H\"\n"
          "#include \"ac_parms.H\"\n"
          //"#include \"debug.h\"\n"
          //"#include <stdio.h>\n"
          //"#include <systemc.h>\n"
          "\n"
          "//Auxiliary: SIGNED\n"
          "#define SIGNED(val,size) \\\n"
          "     ( (val >= (1 << (size-1))) ? val | (0xFFFFFFFF << size) : val )\n"
          "\n"
          );

  //Begin function
  COMMENT(INDENT0,"Begin function","\n");
  fprintf( output,
           "namespace ac_begin {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0);}\n"
           "#define AC_ARGS_begin (ac_stage_list a, unsigned b)\n"
           "#define AC_BEHAVIOR_begin() ac_begin::behavior AC_ARGS_begin\n"
           "\n"
           );

  //End function
  COMMENT(INDENT0,"End function","\n");
  fprintf( output,
           "namespace ac_end {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0);}\n"
           "#define AC_ARGS_end (ac_stage_list a, unsigned b)\n"
           "#define AC_BEHAVIOR_end() ac_end::behavior AC_ARGS_end\n"
           "\n"
           );

  //Generic instruction behavior
  COMMENT(INDENT0,"Generic instruction behavior","\n");
  fprintf( output,
           "#define AC_ARGS_instruction (unsigned ac_instr_size)\n"
           "#define AC_BEHAVIOR_instruction() ac_behavior_instruction AC_ARGS_instruction\n"
           "\n"
           );

  //Arguments for formats and format behaviors
  COMMENT(INDENT0,"Arguments for formats and format behaviors","\n");
  for (pformat = decoder->formats; pformat!= NULL; pformat=pformat->next) {
    fprintf( output, "#define AC_ARGS_%s (unsigned ac_instr_size", pformat->name);

    for (pfield = pformat->fields; pfield!= NULL; pfield=pfield->next) {
      //Field can be signed or unsigned
      if (pfield->sign) fprintf( output, ", signed %s", pfield->name);
      else fprintf( output, ", unsigned %s", pfield->name);
    }

    fprintf( output, ")\n");
    fprintf( output, "#define AC_BEHAVIOR_%s() ac_behavior_%s AC_ARGS_%s\n",
             //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
             pformat->name, pformat->name, pformat->name);
  }
  fprintf( output, "\n");

  //Macros for instruction behaviors
  COMMENT(INDENT0,"Macros for instruction behaviors","\n");
  fprintf( output, "#define ac_behavior(instr)   AC_BEHAVIOR_##instr ()\n\n");
  for( pinstr = decoder->instructions; pinstr!= NULL; pinstr=pinstr->next) {
    fprintf( output, "#define AC_BEHAVIOR_%s() ac_behavior_%s AC_ARGS_%s\n",
             pinstr->name, pinstr->name, pinstr->format);
  }
  fprintf( output, "\n");
  
  //Prototypes for instruction behaviors
  COMMENT(INDENT0,"Prototypes for instruction behaviors","\n");
  fprintf( output, "void ac_behavior(begin);\n");
  fprintf( output, "void ac_behavior(end);\n");
  fprintf( output, "void ac_behavior(instruction);\n");
  for (pformat = decoder->formats; pformat!= NULL; pformat=pformat->next)
    fprintf( output, "void ac_behavior(%s);\n", pformat->name);
  for( pinstr = decoder->instructions; pinstr!= NULL; pinstr=pinstr->next)
    fprintf( output, "void ac_behavior(%s);\n", pinstr->name);
  fprintf( output, "\n");
  
  fprintf( output, "#endif //_ISA_H\n");

  fclose( output); 
}


/*!Create ArchC ISA Implementation File */
/*!Use structures built by the parser.*/
void accs_CreateISAImpl()
{
  extern char *project_name;
  extern int wordsize;

  FILE *output;
  char filename[50];

  snprintf(filename, 50, "%s_isa.cpp", project_name);
  output = fopen(filename, "w");

  print_comment( output, "ISA implementation file.");
  fprintf( output, "#include \"%s_isa.H\"\n\n", project_name);

  //The global and only ISA object
  COMMENT(INDENT0,"The global and only ISA object","\n");
  fprintf( output, "%s_isa ISA;\n\n", project_name);

  /*
    COMMENT(INDENT0,"Constructor","\n");
    fprintf( output, "%s_isa::%2$s_isa() {\n\n", project_name);
    COMMENT(INDENT1, "Decoding structures.","\n");
    accs_EmitDecStruct(output);
    COMMENT(INDENT1, "Building Decoder.","\n");
    fprintf( output,"%sdecoder = CreateDecoder(formats, instructions, %d);\n", INDENT1, wordsize );
    fprintf( output, "};\n");
  */

  fclose( output); 
}


/*!Create ArchC Architecture Dependent Syscalls Header File */
/*!Use structures built by the parser.*/
void accs_CreateArchSyscallHeader()
{
  extern char *project_name;
  FILE *output;
  char filename[50];
  
  snprintf(filename, 50, "%s_syscall.H", project_name);
  output = fopen(filename, "w");

  print_comment( output, "ArchC Architecture Dependent Syscall header file.");

  fprintf(output,
          "#ifndef ARCH_SYSCALL_H\n"
          "#define ARCH_SYSCALL_H\n"
          "\n"
          "#include \"ac_syscall.H\"\n"
          "\n"
          "//%s system calls\n"
          "class %s_syscall : public ac_syscall\n"
          "{\n"
          "public:\n"
          "  void get_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "  void set_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "  void set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size);\n"
          "  int  get_int(int argn);\n"
          "  void set_int(int argn, int val);\n"
          "  void return_from_syscall();\n"
          "  void set_prog_args(int argc, char **argv);\n"
          "};\n"
          "\n"
          "#endif\n"
          //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
          , project_name, project_name);

  fclose( output); 
}


/*!Create ArchC ISA Init File */
/*!Use structures built by the parser.*/
void accs_CreateISAInitImpl()
{
  FILE *output;
  output = fopen("ac_isa_init.cpp", "w");

  print_comment( output, "ArchC ISA Init implementation file.");

  fprintf(output,
          "// This file is empty for compiled simulation\n"
          );

  fclose( output); 
}


/*!Create ArchC Template Implementation File */
/*!Use structures built by the parser.*/
void accs_CreateTemplateImpl()
{
  FILE *output;

  if ( !(output = fopen("ac_template.cpp", "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Template implementation file.");

  fclose( output); 
}




int accs_EmitSyscalls(FILE* output, int j)
{
#define AC_SYSC(NAME,LOCATION) \
          fprintf(output, \
                  "    case " #LOCATION ":\n" \
                  "      PRINT_TRACE; " COUNT_SYSCALLS_INC(NAME) "\n" \
                  "      model_syscall." #NAME "();\n" \
                  "      ac_instr_counter++;\n"); \
          if (ACStatsFlag) fprintf(output, "      ac_sim_stats.syscall_executed++;\n"); \
          if (strcmp( #NAME , "_exit")==0) fprintf( output, "      return;\n"); \
          if (ACDelayFlag) fprintf(output,"      delay::do_assignment();\n"); \
          fprintf(output, "      break;\n\n"); \
          /* Set j to the last location + 1 (find next instruction after reserved addresses) */ \
          if (j < LOCATION + 1) j = LOCATION + 1;
#include "ac_syscall.def"
#undef AC_SYSC

  return j;
}


void accs_EmitInstr(FILE* output, int j)
{
  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decode_table[j]->dec_vector[0]);

  if (PROCESSOR_OPTIMIZATIONS) {
    PROCESSOR_OPTIMIZATIONS_EmitInstr(output, j);
    return;
  }

  //Print instruction to output
  fprintf( output, "    case 0x%x:\n", j);
  accs_EmitInstrExtraTop(output, j, pinstr, 6);
  accs_EmitInstrBehavior(output, j, pinstr, 6);
  accs_EmitInstrExtraBottom(output, j, pinstr, 6);
  fprintf( output, "      break;\n");
  fprintf( output, "\n");
}


void accs_EmitInstrBehavior(FILE* output, int j, ac_dec_instr *pinstr, int indent)
{
  char *args = accs_Fields2Str(decode_table[j]);

  //Print instruction behavior hierarchy to output
  fprintf( output, "%*sac_behavior_instruction(%d);\n", indent, " ", pinstr->size * 8); /* ac_instr_size: in bits */
  fprintf( output, "%*sac_behavior_%s(%d, %s);\n", indent, " ", pinstr->format, pinstr->size * 8, args);
  fprintf( output, "%*sac_behavior_%s(%d, %s);\n", indent, " ", pinstr->name, pinstr->size * 8, args);
}


void accs_EmitInstrExtraTop(FILE* output, int j, ac_dec_instr *pinstr, int indent)
{
  //Print extra instruction actions before behavior to output
  fprintf( output, "%*sPRINT_TRACE;\n", indent, " ");
}


void accs_EmitInstrExtraBottom(FILE* output, int j, ac_dec_instr *pinstr, int indent)
{
  //Print extra instruction actions after behavior to output
  fprintf( output, "%*sac_instr_counter++;\n", indent, " ");

  if (ACStatsFlag)
    fprintf( output, "%*sac_sim_stats.instr_table[%d].count++;\n", indent, " ", pinstr->id);

  if (ACDelayFlag)
    fprintf( output, "%*sdelay::do_assignment();\n", indent, " ");
}


void accs_EmitMakefileExtra(FILE* output)
{
  extern int ACP4Flag, ACOmitFPFlag, ACInlineFlag, ACCompsimFlag;
  fprintf( output, "OPT := $(OPT) ");
  if (ACP4Flag) fprintf( output, "-march=pentium4 ");
  if (ACOmitFPFlag) fprintf( output, "-fomit-frame-pointer ");
  if (ACInlineFlag>0) fprintf( output, "-finline-limit=%d ", ACInlineFlag);
  fprintf( output, "\n");

  fprintf( output, "INLINE := %d\n", (ACInlineFlag?1:0));
  fprintf( output, "ISA_AND_SYSCALL_TOGETHER := %d\n", (ACInlineFlag || PROCESSOR_OPTIMIZATIONS == 2)? 1 : 0);
  fprintf( output, "CFLAGS := $(CFLAGS) $(if $(filter 1,$(INLINE)),-O -finline-functions -fgcse) $(if $(filter 1,$(ISA_AND_SYSCALL_TOGETHER)), -DAC_INLINE) ");
  if (ACCompsimFlag) fprintf( output, "-DAC_COMPSIM");
  fprintf( output, "\n\n");
}


void accs_EmitParmsExtra(FILE* output)
{
  if( PROCESSOR_OPTIMIZATIONS != 0 )
    fprintf( output, "#define  OPT%d \t //!< Indicates what optimization is used for compiled simulation.\n", PROCESSOR_OPTIMIZATIONS);
  if( PROCESSOR_OPTIMIZATIONS == 2 )
    fprintf( output, "#define  NO_NEED_PC_UPDATE \t //!< Indicates that simulator takes care of control flow\n", PROCESSOR_OPTIMIZATIONS);
}


ac_sto_list *accs_FindLoadDevice()
{
  //Already set?
  extern ac_sto_list *load_device;
  if (load_device) return load_device;
  

  extern ac_sto_list *storage_list, *fetch_device;
  ac_sto_list *pstorage;
  load_device = storage_list;

  // COPIED FROM ACPP

  /* Determining which device is gonna be used for fetching instructions */
  if( !fetch_device ){
    //The parser has not determined because there is not an ac_icache obj declared.
    //In this case, look for the object with the lowest (zero) hierarchy level.
    for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next)
      if( pstorage->level == 0 &&  pstorage->type != REG &&  pstorage->type != REGBANK)
        fetch_device = pstorage;

    if( !fetch_device ) { //Couldn't find a fetch device. Error!
      AC_INTERNAL_ERROR("Could not determine a device for fetching.");
      exit(1);
    }
  }

  /* Determining which device is going to be used for loading applications*/
  /* The device used for loading applications must be the one in the highest
     level of a memory hierachy.*/
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
    if(pstorage->level > load_device->level)
      load_device = pstorage;
  }

  /* If there is only one level, which is gonna be zero, then it is the same
     object used for fetching. */
  if( load_device->level ==0 )
    load_device = fetch_device;

  // END: COPIED FROM ACPP

  return load_device;
}


char *accs_Fields2Str(instr_decode_t* decoded_instr)
{
  static char* args = 0;
  char *p_args;

  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decoded_instr->dec_vector[0]);
  ac_dec_field *pfield = FindFormat(decoder->formats, pinstr->format)->fields;

  if (args==0)
    args = malloc(500);

  p_args = args;

  p_args += sprintf( p_args, "%d", decoded_instr->dec_vector[pfield->id]);
  for (pfield = pfield->next; pfield != NULL; pfield = pfield->next) {
    p_args += sprintf( p_args, ", %d", decoded_instr->dec_vector[pfield->id]);
  }
  return args;
}


char *accs_SubstFields(char* expression, int j)
{
  char* new_exp = 0;
  char *p_new_exp;
  char *p_exp, *p_exp_field;
  char *exp;

  ac_dec_instr *pinstr = GetInstrByID(decoder->instructions, decode_table[j]->dec_vector[0]);
  ac_dec_field *pfield = FindFormat(decoder->formats, pinstr->format)->fields;

  exp = strdup(expression);
  new_exp = exp;

  for (; pfield != NULL; pfield = pfield->next) {
    new_exp = accs_SubstValue(exp, pfield->name, decode_table[j]->dec_vector[pfield->id], pfield->sign);
    free(exp);
    exp = new_exp;
  }

  //subst ac_pc also
  new_exp = accs_SubstValue(exp, "ac_pc", j, 0);
  free(exp);

  return new_exp;
}


char *accs_SubstValue(char* str, char* var, unsigned val, int sign)
{
  char* new = 0;
  char *p_new;
  char *p1;
  char *p2=0;

  new = malloc(1000);
  p_new = new;
  p1 = str;

  while (p2 = strstr(p1, var)) {
    strncpy(p_new, p1, p2-p1);
    p_new += p2-p1;
    if (sign) p_new += sprintf(p_new, "%d", val);
    else      p_new += sprintf(p_new, "%u", val);
    p1 = p2 + strlen(var);
  }

  //copy the rest
  strcpy(p_new, p1);
  
  return new;
}


