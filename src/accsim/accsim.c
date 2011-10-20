/*  ArchC Pre-processor generates tools for the described arquitecture
    Copyright (C) 2004  The ArchC Team

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/

/*************************************************************/
/* Accs.c: The ArchC pre-processor for compiled simulation.  */
/* Author: Marcus Bartholomeu                                */
/* Created: 19-03-2003                                       */
/*************************************************************/
//////////////////////////////////////////////////////////
/*!\file accs.c
  \brief The ArchC pre-processor for compiled simulation.
  This file contains functions for compiled simulation.
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
extern int ACMulticoreFlag;
extern int ACAnnulSigFlag;
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

void CreateBhvMacroEmpty();

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

  //MAX
  CreateBhvMacroEmpty();

  //Creates Processor Module Header File 
  //CreateProcessorHeader();
  //!Creates Processor Module Implementation File
  //CreateProcessorImpl();

  //Creating Compiled Simulation Implementation File
  accs_CreateCompsimImpl();
  //Creating Compiled Simulation Header File
  accs_CreateCompsimHeader();
   //Creating Compiled Simulation Header for Region functions
  accs_CreateCompsimHeaderRegions();
  
  //Creating ISA Header File
  accs_CreateISAHeader();
  //Create the template for the .cpp instruction and format behavior file
  //CreateImplTmpl();
  //Creating ISA Init Implementation File
  accs_CreateISAInitImpl();

  //!Creates the ISA Header File    
  //CreateISAHeader();

  //Creating Resources Header File
  //CreateResourceHeader();
  //Creating Resources Reference Header file
  //CreateArchRefHeader();
  //Creating Resources Implementation File
  //CreateResourceImpl();

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
  //CreateDummy(1, "#include \"mips1_parms.H\"\nnamespace ac_begin {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0){};}"); 
  //CreateDummy(2, "#include \"mips1_parms.H\"\nnamespace ac_end {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0){};}"); 


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
    
    if (!ACMulticoreFlag)
	{
		fast_CreateFileResourcesHeader();
		fast_CreateFileResourcesImpl();
	}

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
	if (read(fd, &tmp, 4) == 0){
		AC_ERROR("Read file error //MAX\n");
		close(fd);
		exit(EXIT_FAILURE);
	}
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
    if (read(fd, string_table, convert_endian(4,shdr.sh_size)) == 0){
	AC_ERROR("Read file error //MAX\n");
	close(fd);
	exit(EXIT_FAILURE);	
    }

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
	  if (read(fd, &tmp, 4) == 0){
	  	AC_ERROR("Read file error //MAX\n");
		close(fd);
		exit(EXIT_FAILURE);
	  }
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
      //AC_MSG("Reading application program %s (invalid file?)\n", prog_filename);
      free(instr_mem);
      return EXIT_FAILURE;
    }

    //Found one HEX digit, must read another one
    else if ((fscanf(prog, "%c", &hex2) == EOF) ||
             (!isxdigit(hex2))) {
      //AC_MSG("Reading application program %s (invalid file?)\n", prog_filename);
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

void CreateBhvMacroEmpty()
{
  FILE *output;
  char filename[256];

  // Create empty file
  /* opens behavior macros file */
  sprintf(filename, "%s_bhv_macros.H", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }
  
//  fprintf(output, "#define using /*empty*/\n");
//  fprintf(output, "#define namespace /*empty*/\n");
//  fprintf(output, "#define %s_parms /*empty*/\n", project_name);
//  fprintf(output, "//#define stop() ac_stop\n"); 
//  fprintf(output, "#define stop(EXIT_FAILURE) ac_stop() \n");
 
  fprintf(output, "\n");

  fprintf(output, "#ifdef AC_COMPSIM \n");
  fprintf(output, "#define ac_memory ac_storage\n");
  fprintf(output, "#endif \n\n\n"); 
 
  fprintf(output, "#include \"%s.H\"\n", project_name );
 
  fclose(output);

  //--

  sprintf(filename, "%s_isa_init.cpp", project_name);
   if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  fprintf(output, "//Empty file\n\n");

  fclose(output); 

}

/*!Create ArchC Compiled Simulation Header File */
/*!Use structures built by the parser.*/
void accs_CreateCompsimHeader()
{
  char filename[256];
  FILE *output;
  extern ac_decoder_full *decoder;
  extern ac_dec_field *common_instr_field_list;

  ac_dec_instr *pinstr;
  ac_dec_format *pformat;
  ac_dec_field *pfield;
  int j;

  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;


  sprintf( filename, "%s.H", project_name);
  if ( !(output = fopen( filename, "w"))){
    perror("ArchC could not open output file");
    exit(1);
  }

  print_comment( output, "ArchC Compsim header file.");

  fprintf( output, "#ifndef  _AC_COMPSIM_H\n");
  fprintf( output, "#define  _AC_COMPSIM_H\n");
  fprintf( output, "\n");
  
  fprintf( output, "#include <sys/times.h>\n"); 
  fprintf( output, "#include \"ac_utils.H\"\n");
  fprintf( output, "#include \"%s_parms.H\"\n", project_name);
//  fprintf( output, "#include \"accs_syscall.H\"\n");
  fprintf( output, "#include \"ac_reg.H\"\n");

  if (!ACMulticoreFlag)
	fprintf( output, "#include \"ac_resources.H\"\n");

  fprintf( output, "#include \"ac_storage.H\"\n");
  fprintf( output, "#include \"ac_rtld.H\"\n");
  fprintf( output, "\n");
 
  /*
  //Defining types for the decoded instruction and compiled simulation table
  fprintf( output, "typedef unsigned *decoded_instr_t;\n");
  fprintf( output, "typedef decoded_instr_t *ac_compsim_table_t;\n");
  fprintf( output, "\n\n");
  */
  fprintf( output, "using namespace %s_parms;\n\n", project_name);
/*
  if (!ACMulticoreFlag){
  	  fprintf( output, "unsigned long long ac_instr_counter;\n");
	  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){
	    if( pstorage->type == REGBANK ){
	      fprintf( output, "ac_regbank<%u, ac_word, ac_Dword> %s(\"%s\",%u);\n\n",pstorage->size, pstorage->name, pstorage->name, pstorage->size);
	    }
	  }
  }	
*/


  //Need a trace?
  fprintf( output,
           "//Generate a trace file if defined\n"
           "//(define also in main.cpp)\n"
           "//#define AC_DEBUG\n"
           "\n"
           );

  fprintf( output, "typedef struct {\n");
  fprintf( output, "ac_dynlink::ac_rtld ac_dyn_loader;\n");
  fprintf( output, "} Tref;\n\n");

  fprintf( output, "class %s {\n", project_name);
  fprintf( output, "public: \n" );
  //Function prototypes
  fprintf( output, "\n");
  fprintf( output, "	char *appfilename;\n");
  fprintf( output, "	int ac;\n");
  fprintf( output, "	char **av;\n");
  fprintf( output, "	int ac_stop_flag;\n");
  fprintf( output, "	int ac_exit_status;\n");
  fprintf( output, "	bool ac_annul_sig;\n");
  fprintf( output, "	bool ac_mt_endian;\n");
  fprintf( output, "\n");

  fprintf( output, "	/// dynamic linker/loader into ref\n");
  fprintf( output, "	/// useless for ACCSIM; keep the ACSIM models compatibility\n");
  fprintf (output, "	Tref ref;\n");
  
  if (ACMulticoreFlag)
  {
	  fast_CreateResourcesHeader(output);
 

     if (PROCESSOR_OPTIMIZATIONS > 0)
     {   
	      //Declarations for PROCESSOR_OPTIMIZATIONS
	      fprintf(output, "\n	//Used for optimizations\n");
	      fprintf(output, "	unsigned tmp_pc;\n");
	      fprintf(output, "	unsigned old_pc;\n\n");
      }
  }

  fprintf( output, "\n");
  fprintf( output, "	// Heap pointer.\n");
  fprintf( output, "	unsigned int ac_heap_ptr;\n");
  fprintf( output, "	unsigned ac_start_addr;\n");
  fprintf( output, "\n");
  fprintf( output, "	%s() : \n", project_name);

  if (ACMulticoreFlag)
  {
	fast_CreateResourcesImpl(output);

    if (PROCESSOR_OPTIMIZATIONS > 0)
		  fprintf(output, "	    old_pc(0), \n");

  }

  fprintf( output, "	    ac_exit_status(0),\n"
		   "	    ac_stop_flag (0), \n"
		   "	    ac_mt_endian(0),\n"
		   "	    ac_annul_sig(0)\n"
		   "	    { ac_instr_counter = 0; \n"
		   "	      ac_start_addr = 0; } \n\n");

 
  fprintf( output, "	// Timing structures.\n");
  fprintf( output, "	struct tms ac_run_times;\n");
  fprintf( output, "	clock_t ac_run_start_time;\n");
  fprintf( output, "\n");
  fprintf( output, "	void Execute(int argc, char *argv[]);\n");
  fprintf( output, "	void init(int ac, char **av);\n");
  fprintf( output, "	void start();\n");
  fprintf( output, "	void stop(int status=0);\n");
  fprintf( output, "	void InitStat();\n");
  fprintf( output, "	void PrintStat();\n");
  fprintf( output, "\n");

  fprintf( output, "	void ac_wait(){\n");
  fprintf( output, "		ac_wait_sig = 1;\n");
  fprintf( output, "	};\n");
  fprintf( output, "\n");
  fprintf( output, "	void ac_release(){\n");
  fprintf( output, "		ac_wait_sig = 0;\n");
  fprintf( output, "	};\n");
  fprintf( output, "\n");

  fprintf( output, "	void ac_annul(){\n"
		   "		ac_annul_sig = 1;\n"
		   "	};\n\n"); 

 
  fprintf( output, "	// ac_syscall\n");
  fprintf(output,
	  "	#define AC_SYSC(NAME,LOCATION) void NAME();\n"
	  "	#include <ac_syscall.def>\n"
	  "	#undef AC_SYSC\n\n"
          "	void get_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "	void set_buffer(int argn, unsigned char* buf, unsigned int size);\n"
          "	void set_buffer_noinvert(int argn, unsigned char* buf, unsigned int size);\n"
          "	int  get_int(int argn);\n"
          "	void set_int(int argn, int val);\n"
          "	void return_from_syscall();\n"
          "	void set_prog_args(int argc, char **argv);\n"
	  "	void set_pc(unsigned val);\n"
	  "	void set_return(unsigned val);\n"
	  "	unsigned get_return();\n"
	  "	int *get_syscall_table();\n");
	  //"	int process_syscall(int syscall);");

  for (j=0; j <= ((prog_size_bytes-1) >> REGION_SIZE); j++) {
    	fprintf(output, "	void Region%d();\n" , j, j);
      }

  fprintf( output, "\n");

  //Generic instruction behavior
  COMMENT(INDENT[3],"Generic instruction behavior","\n");

  fprintf( output,
           "void ac_behavior_instruction (unsigned ac_instr_size " );

  /* common_instr_field_list has the list of fields for the generic instruction. */
  for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
      fprintf(output, ", ");
      if( pfield->sign )
        fprintf(output, "int %s", pfield->name);
      else 
        fprintf(output, "unsigned int %s", pfield->name);
    }    
    fprintf(output, ");\n\n");

/*
  fprintf( output,
           "	void ac_behavior_instruction (unsigned ac_instr_size);\n"
           "\n"
           );
*/

  /* begin & end */
  fprintf(output, "%svoid ac_behavior_begin();\n", INDENT[4]);
  fprintf(output, "%svoid ac_behavior_end();\n\n", INDENT[4]);

   //Arguments for formats and format behaviors
  COMMENT(INDENT[3],"Arguments for formats and format behaviors","\n");
  for (pformat = decoder->formats; pformat!= NULL; pformat=pformat->next) {
    fprintf( output, "	void ac_behavior_%s (unsigned ac_instr_size", pformat->name);

    for (pfield = pformat->fields; pfield!= NULL; pfield=pfield->next) {
      //Field can be signed or unsigned
      if (pfield->sign) fprintf( output, ", signed %s", pfield->name);
      else fprintf( output, ", unsigned %s", pfield->name);
    }
    fprintf( output, ");\n");
  }
  fprintf( output, "\n\n");

  //Macros for instruction behaviors  
  COMMENT(INDENT[3], "Instruction behaviors","\n");
  for (pinstr = decoder->instructions; pinstr != NULL; pinstr = pinstr->next) {
      for (pformat = decoder->formats;
	   (pformat != NULL) && strcmp(pinstr->format, pformat->name);
	   pformat = pformat->next);
      fprintf(output, "	void ac_behavior_%s (unsigned ac_instr_size, ", pinstr->name);
      for (pfield = pformat->fields; pfield != NULL; pfield = pfield->next) {
	if (pfield -> sign)
	  fprintf(output, "int %s", pfield->name);
	else
	  fprintf(output, "unsigned int %s", pfield->name);
	if (pfield->next != NULL)
	  fprintf(output, ", ");
      }
      fprintf(output, ");\n");
    }

  fprintf( output, "\n");
   
  fprintf( output, "};\n");
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
//    fprintf( output, "#include \"archc_cs.H\"\n");

    fprintf( output, "#include \"%s_isa.H\"\n", project_name);

    if (rblock == 0) {
      fprintf( output, "#include \"ac_prog_regions.H\"\n");
    }

    fprintf( output, "\n");
//    fprintf( output, "#define AC_ERROR( msg )    std::cerr<< \"ArchC ERROR: \" << msg  <<'\\n'\n");
//    fprintf( output, "#define AC_SAY( msg )      std::cerr<< \"ArchC: \" << msg  <<'\\n'\n");
//    fprintf( output, "\n");

      if (ACABIFlag) {
        fprintf( output, "#include \"%s_syscall_macros.H\"\n", project_name);
      }

    // Inlines for command-line option "-i"
    fprintf( output, "#ifdef AC_INLINE\n");
    fprintf( output, "\n");
    fprintf( output, "//ISA instructions are compiled together for inline to work\n");
    fprintf( output, "#include \"%s_isa.cpp\"\n", project_name);
    fprintf( output, "\n");
//  if (rblock == 0) {
//    if (ACABIFlag) {
        fprintf( output, "//Syscalls are compiled together for inline to work\n");
        fprintf( output, "#include \"%s_syscall.cpp\"\n", project_name);
        fprintf( output, "\n");
//      }
//    }
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
//      if (ACABIFlag)       fprintf(output, "%s_syscall model_syscall;\n\n", project_name);
    }

    //Declarations for PROCESSOR_OPTIMIZATIONS
    if ((PROCESSOR_OPTIMIZATIONS > 0)&&(ACMulticoreFlag == 0)) {
      fprintf(output, "	//Used for optimizations\n");
      fprintf(output, "	unsigned tmp_pc;\n\n");
    }



    // @@@@@@@@@@@@@@@@@ kernel of compiled simulation @@@@@@@@@@@@@@@@@@@@@@@

    for (i=rblock*REGION_BLOCK_SIZE;
         ((i < (rblock+1)*REGION_BLOCK_SIZE) && (i <= ((prog_size_bytes-1) >> REGION_SIZE)));
         i++) {
      int end_region = (prog_size_bytes < ((i+1) << REGION_SIZE)) ? prog_size_bytes : ((i+1) << REGION_SIZE);
    
      // Region function start
      fprintf(output, "void %s::Region%d() {\n", project_name, i);
      fprintf(output,
              "\n"
              "  while (1) {\n"
              "    switch((int)ac_pc) {\n"
              "\n"
              );


      // KERNEL: emit instructions
      for (j = (i << REGION_SIZE); j < end_region; j++) {

        if ((ACABIFlag) && (j==60)) {  // Special addresses for ArchC system calls
          j = accs_EmitSyscalls(output, j);
        }

        if ((decode_table[j]) && (decode_table[j]->dec_vector)) {
          accs_EmitInstr(output, j);
        }
      }


      // Write invalid PC section and end Region function
      if ((PROCESSOR_OPTIMIZATIONS)&&(ACMulticoreFlag==1))
      	fprintf( output, "      ac_instr_counter += (0x%x - old_pc)/4 + 1;\n", j-4);      
//      else
//     	 if (PROCESSOR_OPTIMIZATIONS == 2)
//     	 	fprintf( output, "      ac_instr_counter += (0x%x - ac_pc)/4 + 1;\n", j-4);      


      fprintf( output,
               "      //certify to change region on fall-through\n"
               "      ac_pc = %#x;\n"
	       "      break;\n"
               "\n"
               "    default:\n"
               , ((i+1) << REGION_SIZE));
      if ((PROCESSOR_OPTIMIZATIONS)&&(ACMulticoreFlag==1)) 
      	fprintf( output, "      old_pc = ac_pc;\n");
      if (i == EXIT_ADDRESS>>REGION_SIZE) fprintf( output, "      if (ac_pc == %d) return;\n", EXIT_ADDRESS);
      fprintf( output,
               "      if ((ac_pc >= %d) && (ac_pc < %d)) {\n"
               "        AC_ERROR(\"ac_pc=0x\" << hex << int(ac_pc) << \" points to an non-decoded memory location.\" << endl);\n"
               "        stop(EXIT_FAILURE);\n"
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

      fprintf(output, "void %s::Execute(int argc, char *argv[]) {\n\n", project_name);
      if (ACABIFlag) {fprintf(output, "  set_prog_args(argc, argv);\n\n");}
      fprintf(output,
             // "  extern int ac_stop_flag;\n"
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
              "      stop(EXIT_FAILURE);\n"
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

	fprintf(output, "#include <ac_sighandlers.H>\n\n");	
      	fprintf(output, "void %s::init(int ac, char **av){\n", project_name);
      	fprintf(output, "	this->ac = ac;\n");
	fprintf(output, "	this->av = av;\n");
      	fprintf(output, "	ac_init_opt( ac, av);\n");
      	fprintf(output, "	std::cerr << endl;\n");
      	fprintf(output, "	AC_SAY(\"Compiled simulator for program: \" << appfilename);\n");
      	fprintf(output, "   #ifdef USE_GDB\n");
	fprintf(output, "	if (gdbstub && !gdbstub->is_disabled())\n");
	fprintf(output, "		gdbstub->connect();\n");
	fprintf(output, "   #endif /* USE_GDB */\n");
	fprintf(output, "	ac_pc = ac_start_addr;\n");
	fprintf(output, "	ac_behavior_begin();\n");
	fprintf(output, "	cerr << \"ArchC: -------------------- Starting Simulation --------------------\" << endl;\n");
	fprintf(output, "	InitStat();\n");
	fprintf(output, "	signal(SIGINT, sigint_handler);\n");
	fprintf(output, "	signal(SIGTERM, sigint_handler);\n");
	fprintf(output, "	signal(SIGSEGV, sigsegv_handler);\n");
	fprintf(output, "	signal(SIGUSR1, sigusr1_handler);\n");
	fprintf(output, "    #ifdef USE_GDB\n");
	fprintf(output, "	signal(SIGUSR2, sigusr2_handler);\n");
	fprintf(output, "    #endif\n");
      	fprintf(output, "}\n\n");

      	fprintf(output, "void %s::start(){\n", project_name);
	fprintf(output, "	ac_pc = ac_start_addr;\n");
	fprintf(output, "	char **new_argv = new char*[ac];\n");
	fprintf(output, "	new_argv[0] = appfilename;\n");
	fprintf(output, "	for (int i=1; i<ac; i++) new_argv[i] = av[i];\n");
	fprintf(output, "	Execute(ac, new_argv);\n");
	fprintf(output, "	delete [] new_argv;\n");
	fprintf(output, "}\n\n");

	fprintf(output, "void %s::stop(int status){\n", project_name);
	fprintf(output, "	cerr << \"ArchC: -------------------- Simulation Finished --------------------\" << endl;\n");
	fprintf(output, "	ac_behavior_end();\n");
	fprintf(output, "	ac_stop_flag = 1;\n");
	fprintf(output, "	ac_exit_status = status;\n");
	fprintf(output, "	ac_pc = ~0;\n");
	fprintf(output, "	}\n\n");

	fprintf(output, "void %s::InitStat(){\n", project_name);
	fprintf(output, "	ac_run_start_time = ::times(&ac_run_times);\n");
	fprintf(output, "}\n\n");
	
  	fprintf(output, "void %s::PrintStat(){\n", project_name);
	fprintf(output, "	clock_t ac_run_real;\n\n");
	fprintf(output, "	//Print statistics\n");
	fprintf(output, "	fprintf(stderr, \"ArchC: Simulation statistics\\n\");\n");
	fprintf(output, "	ac_run_real = ::times(&ac_run_times) - ac_run_start_time;\n");
	fprintf(output, "	fprintf(stderr, \"   Times: %%ld.%%02ld user, %%ld.%%02ld system, %%ld.%%02ld real\\n\",\n");
	fprintf(output, "		ac_run_times.tms_utime / 100, ac_run_times.tms_utime %% 100,\n");
	fprintf(output, "		ac_run_times.tms_stime / 100, ac_run_times.tms_stime %% 100,\n");
	fprintf(output, "		ac_run_real / 100, ac_run_real %% 100\n");
	fprintf(output, "		);\n\n");
	fprintf(output, "	fprintf(stderr, \"    Number of instructions executed: %%llu\\n\", ac_instr_counter);\n");
	fprintf(output, "	if (ac_run_times.tms_utime > 5) {\n");
	fprintf(output, "		double ac_mips = (ac_instr_counter * 100) / ac_run_times.tms_utime;\n");
	fprintf(output, "		fprintf(stderr, \"    Simulation speed: %%.2f K instr/s\\n\", ac_mips/1000);\n");
	fprintf(output, "	} else {\n");
	fprintf(output, "		fprintf(stderr, \"    Simulation speed: (too fast to be precise)\\n\");\n");
	fprintf(output, "	}\n");
	fprintf(output, "} \n\n");

	// inserindo codigos do ac_syscall.cpp 
	// Motivo: Velocidade na execucao e visibilidade dos atributos
	fprintf(output, 
			"#ifdef DEBUG\n"
			"#  define DEBUG_SYSCALL(name) AC_RUN_MSG(\"@@@@@ syscall: \" name \" @@@@@\\n\")\n"
			"#else\n"
			"#  define DEBUG_SYSCALL(name)\n"				
			"#endif\n"							
			"\n"									
			"\n"										
			"// Fix incompatibility from NewLib flags and Linux flags\n"												
			"\n"											
			"#define NEWLIB_O_RDONLY          0x0000			\n"								
			"#define NEWLIB_O_WRONLY          0x0001			\n"									
			"#define NEWLIB_O_RDWR            0x0002			\n"											
			"#define NEWLIB_O_APPEND          0x0008			\n"										
			"#define NEWLIB_O_CREAT           0x0200			\n"								
			"#define NEWLIB_O_TRUNC           0x0400			\n"							
			"#define NEWLIB_O_EXCL            0x0800			\n"										
			"#define NEWLIB_O_NOCTTY          0x8000			\n"										
			"#define NEWLIB_O_NONBLOCK        0x4000			\n"												
			"								\n"											
			"#define CORRECT_O_RDONLY             00			\n"										
			"#define CORRECT_O_WRONLY             01			\n"									
			"#define CORRECT_O_RDWR               02			\n"										
			"#define CORRECT_O_CREAT            0100			\n"								
			"#define CORRECT_O_EXCL             0200			\n"									
			"#define CORRECT_O_NOCTTY           0400			\n"										
			"#define CORRECT_O_TRUNC           01000			\n"									
			"#define CORRECT_O_APPEND          02000			\n"									
			"#define CORRECT_O_NONBLOCK        04000			\n"									
			"\n"	
			"void correct_flags( int* val )\n"
			"{\n"
			"  int f = *val;\n"
			"  int flags = 0;\n"
			"\n"
			"  if( f &  NEWLIB_O_RDONLY )\n"
			"    flags |= CORRECT_O_RDONLY;\n"
			"  if( f &  NEWLIB_O_WRONLY )\n"
			"    flags |= CORRECT_O_WRONLY;\n"
			"  if( f &  NEWLIB_O_RDWR )\n"
			"    flags |= CORRECT_O_RDWR;\n"
			"  if( f & NEWLIB_O_CREAT )\n"
			"    flags |= CORRECT_O_CREAT;\n"
			"  if( f & NEWLIB_O_EXCL )\n"
			"    flags |= CORRECT_O_EXCL;\n"
			"  if( f & NEWLIB_O_NOCTTY )\n"
			"    flags |= CORRECT_O_NOCTTY;\n"
			"  if( f & NEWLIB_O_TRUNC )\n"
			"    flags |= CORRECT_O_TRUNC;\n"
			"  if( f & NEWLIB_O_APPEND )\n"
			"    flags |= CORRECT_O_APPEND;\n"
			"  if( f & NEWLIB_O_NONBLOCK )\n"
			"    flags |= CORRECT_O_NONBLOCK;\n"
			"\n"
			"  *val = flags;\n"
			"}\n"
			"\n"
			"\n" );


fprintf( output, 
			"#include <sys/utsname.h> \n" 
			"#include <sys/uio.h>\n"
			"\n"
			"#define SET_BUFFER_CORRECT_ENDIAN(reg, buf, size)                       \\\n"
			"  do {                                                                  \\\n"
			"    unsigned char *ptr = (unsigned char*) buf;                          \\\n"
			"    for (int ndx = 0; ndx < (size); ndx += sizeof(ac_word)) {           \\\n"
			"      *((ac_word *)(ptr + ndx)) = (ac_word)                             \\\n"
			"        convert_endian(sizeof(ac_word), (unsigned) *((ac_word *)(ptr + ndx)), \\\n"
			"                       ac_mt_endian);                               \\\n"
			"  }                                                                     \\\n"
			"    set_buffer((reg), ptr, (size));                                     \\\n"
			"  } while(0)\n"
			"\n"
			"#define CORRECT_ENDIAN(word, size) (convert_endian((size),              \\\n"
			"                                                   (word), ac_mt_endian))\n"
			"\n"
			"#define CORRECT_STAT_STRUCT(buf)                                          \\\n"
			"  do{                                                                   \\\n"
			"    buf.st_dev     = CORRECT_ENDIAN(buf.st_dev, sizeof(dev_t));         \\\n"
			"    buf.st_ino     = CORRECT_ENDIAN(buf.st_ino, sizeof(ino_t));         \\\n"
			"    buf.st_mode    = CORRECT_ENDIAN(buf.st_mode, sizeof(mode_t));       \\\n"
			"    buf.st_nlink   = CORRECT_ENDIAN(buf.st_nlink, sizeof(nlink_t));     \\\n"
			"    buf.st_uid     = CORRECT_ENDIAN(buf.st_uid, sizeof(uid_t));         \\\n"
			"    buf.st_gid     = CORRECT_ENDIAN(buf.st_gid, sizeof(gid_t));         \\\n"
			"    buf.st_rdev    = CORRECT_ENDIAN(buf.st_rdev, sizeof(dev_t));        \\\n"
			"    buf.st_size    = CORRECT_ENDIAN(buf.st_size, sizeof(off_t));        \\\n"
			"    buf.st_blksize = CORRECT_ENDIAN(buf.st_blksize, sizeof(blksize_t)); \\\n"
			"    buf.st_blocks  = CORRECT_ENDIAN(buf.st_blocks, sizeof(blkcnt_t));   \\\n"
			"    buf.st_atime   = CORRECT_ENDIAN(buf.st_atime, sizeof(time_t));      \\\n"
			"    buf.st_mtime   = CORRECT_ENDIAN(buf.st_mtime, sizeof(time_t));      \\\n"
			"    buf.st_ctime   = CORRECT_ENDIAN(buf.st_ctime, sizeof(time_t));      \\\n"
			"  } while(0)\n"
			"\n"
			"\n")/
#if 0
			"\n"
			"int %s::process_syscall(int syscall) {\n"
			"  const int *sctbl = get_syscall_table();\n"
			"  \n"
			"  if (sctbl == NULL)\n"
			"     return -1;\n"
			"\n"
			"  if (syscall == sctbl[0]) {    // restart_syscall\n"
			"\n"
			"  } else if (syscall == sctbl[1]) { // exit\n"
			"    DEBUG_SYSCALL(\"exit\");\n"
			"    int ac_exit_status = get_int(0);\n"
			"#ifdef USE_GDB\n"
			"    if (ref.get_gdbstub()) (ref.get_gdbstub())->exit(ac_exit_status);\n"
			"#endif /* USE_GDB */\n"
			"    stop(ac_exit_status);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[2]) { // fork\n"
			"\n"
			"  } else if (syscall == sctbl[3]) { // read\n"
			"    DEBUG_SYSCALL(\"read\");\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"    if (!flush_cache()) return;\n"
			"#endif  \n"
			"    int fd = get_int(0);\n"
			"    unsigned count = get_int(2);\n"
			"    unsigned char *buf = (unsigned char*) malloc(count);\n"
			"    int ret = ::read(fd, buf, count);\n"
			"    set_buffer(1, buf, ret);\n"
			"    set_int(0, ret);\n"
			"    free(buf);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[4]) { // write\n"
			"    DEBUG_SYSCALL(\"write\");\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"    if (!flush_cache()) return;\n"
			"#endif\n"
			"    int fd = get_int(0);\n"
			"    unsigned count = get_int(2);\n"
			"    unsigned char *buf = (unsigned char*) malloc(count);\n"
			"    get_buffer(1, buf, count);\n"
			"    int ret = ::write(fd, buf, count);\n"
			"    set_int(0, ret);\n"
			"    free(buf);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[5]) { // open\n"
			"    DEBUG_SYSCALL(\"open\");\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"    if (!flush_cache()) return;\n"
			"#endif\n"
			"    unsigned char pathname[100];\n"
			"    get_buffer(0, pathname, 100);\n"
			"    int flags = get_int(1);\n"
			"    int mode = get_int(2);\n"
			"    int ret = ::open((char*)pathname, flags, mode);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[6]) { // close\n"
			"    DEBUG_SYSCALL(\"close\");\n"
			"    int fd = get_int(0);\n"
			"    int ret;\n"
			"    // Silently ignore attempts to close standard streams (newlib may try to do so when exiting)\n"
			"    if (fd == STDIN_FILENO || fd == STDOUT_FILENO || fd == STDERR_FILENO)\n"
			"      ret = 0;\n"
			"    else\n"
			"      ret = ::close(fd);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[7]) { // creat\n"
			"    DEBUG_SYSCALL(\"creat\");\n"
			"    unsigned char pathname[100];\n"
			"    get_buffer(0, pathname, 100);\n"
			"    int mode = get_int(1);\n"
			"    int ret = ::creat((char*)pathname, mode);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[8]) { // time\n"
			"    DEBUG_SYSCALL(\"time\");\n"
			"    time_t param;\n"
			"    time_t ret = ::time(&param);\n"
			"    if (get_int(0) != 0 && ret != (time_t)-1)\n"
			"      SET_BUFFER_CORRECT_ENDIAN(0, (unsigned char *)&param,(unsigned) sizeof(time_t));\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[9]) { // lseek\n"
			"    DEBUG_SYSCALL(\"lseek\");\n"
			"    off_t offset = get_int(1);\n"
			"    int whence = get_int(2);\n"
			"    int fd = get_int(0);\n"
			"    int ret;\n"
			"    ret = ::lseek(fd, offset, whence);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[10]) { // getpid\n"
			"    DEBUG_SYSCALL(\"getpid\");\n"
			"    pid_t ret = getpid();\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[11]) { // access\n"
			"    DEBUG_SYSCALL(\"access\");\n"
			"    unsigned char pathname[100];\n"
			"    get_buffer(0, pathname, 100);\n"
			"    int mode = get_int(1);\n"
			"    int ret = ::access((char*)pathname, mode);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[12]) { // kill\n"
			"    DEBUG_SYSCALL(\"kill\");\n"
			"    set_int(0, 0); \n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[13]) { // dup\n"
			"    DEBUG_SYSCALL(\"dup\");\n"
			"    int fd = get_int(0);\n"
			"    int ret = dup(fd);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[14]) { // times\n"
			"    DEBUG_SYSCALL(\"times\");\n"
			"    struct tms buf;\n"
			"    clock_t ret = ::times(&buf);\n"
			"    if (ret != (clock_t)-1)\n"
			"      SET_BUFFER_CORRECT_ENDIAN(0, (unsigned char*)&buf, \n"
			"                                (unsigned)sizeof(struct tms));\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[15]) { // brk\n"
			"    DEBUG_SYSCALL(\"brk\");\n"
			"    int ptr = get_int(0);\n"
			"    set_int(0, ref.ac_dyn_loader.mem_map.brk((Elf32_Addr)ptr));\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[16]) { // mmap\n"
			"    DEBUG_SYSCALL(\"mmap\");\n"
			"    // Supports only anonymous mappings\n"
			"    int flags = get_int(3);\n"
			"    Elf32_Addr addr = get_int(0);\n"
			"    Elf32_Word size = get_int(1);\n"
			"    if ((flags & 0x20) == 0) { // Not anonymous\n"
			"      set_int(0, -EINVAL);\n"
			"    } else {\n"
			"      set_int(0, ref.ac_dyn_loader.mem_map.mmap_anon(addr, size));\n"
			"    }\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[17]) { // munmap\n"
			"    DEBUG_SYSCALL(\"munmap\");\n"
			"    Elf32_Addr addr = get_int(0);\n"
			"    Elf32_Word size = get_int(1);\n"
			"    if (ref.ac_dyn_loader.mem_map.munmap(addr, size))\n"
			"      set_int(0, 0);\n"
			"    else\n"
			"      set_int(0, -EINVAL);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[18]) { // stat\n"
			"    DEBUG_SYSCALL(\"stat\");\n"
			"    unsigned char pathname[256];\n"
			"    get_buffer(0, pathname, 256);\n"
			"    struct stat buf;\n"
			"    int ret = ::stat((char *)pathname, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[19]) { // lstat\n"
			"    DEBUG_SYSCALL(\"lstat\");\n"
			"    unsigned char pathname[256];\n"
			"    get_buffer(0, pathname, 256);\n"
			"    struct stat buf;\n"
			"    int ret = ::lstat((char *)pathname, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[20]) { // fstat\n"
			"    DEBUG_SYSCALL(\"fstat\");\n"
			"    int fd = get_int(0);\n"
			"    struct stat buf;\n"
			"    int ret = ::fstat(fd, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[21]) { // uname\n"
			"    DEBUG_SYSCALL(\"uname\");\n"
			"    struct utsname *buf = (struct utsname*) malloc(sizeof(utsname));\n"
			"    int ret = ::uname(buf);\n"
			"    set_buffer(0, (unsigned char *) buf, sizeof(utsname));\n"
			"    free(buf);\n"
			"    set_int(0, ret);\n"
			"    return 0; \n"
			"\n"
			"  } else if (syscall == sctbl[22]) { // _llseek\n"
			"    DEBUG_SYSCALL(\"_llseek\");\n"
			"    unsigned fd = get_int(0);\n"
			"    unsigned long offset_high = get_int(1);\n"
			"    unsigned long offset_low = get_int(2);\n"
			"    off_t ret_off;\n"
			"    int ret;\n"
			"    unsigned whence = get_int(4);\n"
			"    if (offset_high == 0) {\n"
			"      ret_off = ::lseek(fd, offset_low, whence);\n"
			"      if (ret_off >= 0) {\n"
			"	loff_t result = ret_off;\n"
			"	SET_BUFFER_CORRECT_ENDIAN(3, (unsigned char*)&result,\n"
			"                                  (unsigned) sizeof(loff_t));\n"
			"	ret = 0;\n"
			"      } else {\n"
			"	ret = -1;\n"
			"      }\n"
			"    } else {\n"
			"      ret = -1;\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0; \n"
			"\n"
			"  } else if (syscall == sctbl[23]) { // readv\n"
			"    DEBUG_SYSCALL(\"readv\");\n"
			"    int ret;\n"
			"    int fd = get_int(0);\n"
			"    int iovcnt = get_int(2);\n"
			"    int *addresses = (int *) malloc(sizeof(int)*iovcnt);\n"
			"    struct iovec *buf = (struct iovec *) malloc(sizeof(struct iovec)*iovcnt);\n"
			"    get_buffer(1, (unsigned char *) buf, sizeof(struct iovec)*iovcnt);\n"
			"    for (int i = 0; i < iovcnt; i++) {\n"
			"      addresses[i] = (int) buf[i].iov_base;\n"
			"      unsigned char *tmp = (unsigned char *) malloc(buf[i].iov_len);\n"
			"      buf[i].iov_base = (void *)tmp;\n"
			"    }\n"
			"    ret = ::readv(fd, buf, iovcnt);\n"
			"    for (int i = 0; i < iovcnt; i++) {\n"
			"      set_int(1, addresses[i]);\n"
			"      set_buffer(1, (unsigned char *)buf[i].iov_base, buf[i].iov_len);\n"
			"      free (buf[i].iov_base);\n"
			"    }\n"
			"    free(addresses);\n"
			"    free(buf);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[24]) { // writev\n"
			"    DEBUG_SYSCALL(\"writev\");\n"
			"    int ret;\n"
			"    int fd = get_int(0);\n"
			"    int iovcnt = get_int(2);\n"
			"    struct iovec *buf = (struct iovec *) malloc(sizeof(struct iovec)*iovcnt);\n"
			"    get_buffer(1, (unsigned char *) buf, sizeof(struct iovec)*iovcnt);\n"
			"    for (int i = 0; i < iovcnt; i++) {\n"
			"      unsigned char *tmp;\n"
			"      buf[i].iov_base = (void *) \n"
			"        CORRECT_ENDIAN((unsigned) buf[i].iov_base, sizeof(void *));\n"
			"      buf[i].iov_len  = CORRECT_ENDIAN(buf[i].iov_len, sizeof(size_t));\n"
			"      set_int(1, (int) buf[i].iov_base);\n"
			"      tmp = (unsigned char *) malloc(buf[i].iov_len);\n"
			"      buf[i].iov_base = (void *)tmp;\n"
			"      get_buffer(1, tmp, buf[i].iov_len);\n"
			"    }\n"
			"    ret = ::writev(fd, buf, iovcnt);\n"
			"    for (int i = 0; i < iovcnt; i++) {\n"
			"      free (buf[i].iov_base);\n"
			"    }\n"
			"    free(buf);\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[25]) { // mmap2\n"
			"    return process_syscall(sctbl[16]); //redirect to mmap\n"
			"\n"
			"  } else if (syscall == sctbl[26]) { // stat64\n"
			"    DEBUG_SYSCALL(\"stat64\");\n"
			"    unsigned char pathname[256];\n"
			"    get_buffer(0, pathname, 256);\n"
			"    struct stat64 buf;\n"
			"    int ret = ::stat64((char *)pathname, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat64));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[27]) { // lstat64\n"
			"    DEBUG_SYSCALL(\"lstat64\");\n"
			"    unsigned char pathname[256];\n"
			"    get_buffer(0, pathname, 256);\n"
			"    struct stat64 buf;\n"
			"    int ret = ::lstat64((char *)pathname, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat64));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[28]) { // fstat64\n"
			"    DEBUG_SYSCALL(\"fstat64\");\n"
			"    int fd = get_int(0);\n"
			"    struct stat64 buf;\n"
			"    int ret = ::fstat64(fd, &buf);\n"
			"    if (ret >= 0) {\n"
			"      CORRECT_STAT_STRUCT(buf);\n"
			"      set_buffer(1, (unsigned char*)&buf, sizeof(struct stat64));\n"
			"    }\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[29]) { // getuid32\n"
			"    DEBUG_SYSCALL(\"getuid32\");\n"
			"    uid_t ret = ::getuid();\n"
			"    set_int(0, (int)ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[30]) { // getgid32\n"
			"    DEBUG_SYSCALL(\"getgid32\");\n"
			"    gid_t ret = ::getgid();\n"
			"    set_int(0, (int)ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[31]) { // geteuid32\n"
			"    DEBUG_SYSCALL(\"geteuid32\");\n"
			"    uid_t ret = ::geteuid();\n"
			"    set_int(0, (int)ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[32]) { // getegid32\n"
			"    DEBUG_SYSCALL(\"getegid32\");\n"
			"    gid_t ret = ::getegid();"
			"    set_int(0, (int)ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[33]) { // fcntl64\n"
			"    DEBUG_SYSCALL(\"fcntl64\");\n"
			"    int ret = -EINVAL;\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"\n"
			"  } else if (syscall == sctbl[34]) { // exit_group\n"
			"    DEBUG_SYSCALL(\"exit_group\");\n"
			"    int ret = -EINVAL;\n"
			"    set_int(0, ret);\n"
			"    return 0;\n"
			"  }\n"
			"\n"
			"  /* Default case */\n"
			"  set_int(0, -EINVAL);\n"
			"  return -1;\n"
			"}\n"
			"\n\n", project_name);
#endif
			fprintf( output, "#undef AC_RUN_ERROR\n"
			"#define AC_RUN_ERROR std::cerr << \"ArchC Runtime error (ac_pc=\" << std::hex << ac_pc << std::dec << \"; ac_instr_counter=\" << ac_instr_counter << \"): \" \n"
		    	"#define AC_SYSCALL void %s\n"
			"#include <ac_syscall.H>\n\n", project_name);
			
#if 0
			"//! Processor independent functions (syscalls)\n"
			"\n"
			"void %s::open()\n"
			"{\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"  if (!flush_cache()) return;\n"
			"#endif\n"
			"  DEBUG_SYSCALL(\"open\");\n"
			"  unsigned char pathname[100];\n"
			"  get_buffer(0, pathname, 100);\n"
			"  int flags = get_int(1); correct_flags(&flags);\n"
			"  int mode = get_int(2);\n"
			"  int ret = ::open((char*)pathname, flags, mode);\n"
			"//   if (ret == -1) {\n"
			"//     AC_RUN_ERROR(\"System Call open (file '%%s'): %%s\\n\", pathname, strerror(errno));\n"
			"//     exit(EXIT_FAILURE);\n"
			"//   }\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::creat()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"creat\");\n"
			"  unsigned char pathname[100];\n"
			"  get_buffer(0, pathname, 100);\n"
			"  int mode = get_int(1);\n"
			"  int ret = ::creat((char*)pathname, mode);\n"
			"  if (ret == -1) {\n"
			"    AC_RUN_ERROR(\"System Call creat (file '%%s'): %%s\\n\", pathname,strerror(errno));\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::close()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"close\");\n"
			"  int fd = get_int(0);\n"
			"  int ret = ::close(fd);\n"
			"  if (ret == -1) {\n"
			"    AC_RUN_ERROR(\"System Call close (fd %%d): %%s\\n\", fd, strerror(errno));\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::read()\n"
			"{\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"  if (!flush_cache()) return;\n"
			"#endif  \n"
			"  DEBUG_SYSCALL(\"read\");\n"
			"  int fd = get_int(0);\n"
			"  unsigned count = get_int(2);\n"
			"  unsigned char *buf = (unsigned char*) malloc(count);\n"
			"  int ret = ::read(fd, buf, count);\n"
			"  if (ret == -1) {\n"
			"    AC_RUN_ERROR(\"System Call read (fd %%d): %%s\\n\", fd, strerror(errno));\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"  set_buffer(1, buf, ret);\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"  free(buf);\n"
			"}\n"
			"\n"
			"\n"
			"void %s::write()\n"
			"{\n"
			"#ifdef AC_MEM_HIERARCHY\n"
			"  if (!flush_cache()) return;\n"
			"#endif\n"
			"  DEBUG_SYSCALL(\"write\");\n"
			"  int fd = get_int(0);\n"
			"  unsigned count = get_int(2);\n"
			"  unsigned char *buf = (unsigned char*) malloc(count);\n"
			"  get_buffer(1, buf, count);\n"
			"  int ret = ::write(fd, buf, count);\n"
			"  if (ret == -1) {\n"
			"    AC_RUN_ERROR(\"System Call write (fd %%d): %%s\\n\", fd, strerror(errno));\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"  free(buf);\n"
			"}\n"
			"\n"
			"\n"
			"void %s::isatty()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"isatty\");\n"
			"  int desc = get_int(0);\n"
			"  int ret = ::isatty(desc);\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::sbrk()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"sbrk\");\n"
			"  unsigned int base = ac_heap_ptr;\n"
			"  unsigned int increment = get_int(0);\n"
			"  ac_heap_ptr += increment;\n"
			"\n"
			"  // Test if there is enough space in the target memory \n"
			"  // OBS: 1kb is reserved at the end of memory to command line parameters\n"
			"  if (ac_heap_ptr > AC_RAMSIZE-1024) {\n"
			"    // Show error only once\n"
			"    static bool show_error = true;\n"
			"    if (show_error) {\n"
			"      AC_WARN(\"Target application failed to allocate \" << increment <<\n"
			"               \" bytes: heap(=\" << ac_heap_ptr << \") > ramsize(=\" <<\n"
			"               AC_RAMSIZE << \")\");\n"
			"      AC_WARN(\"If target application does not treat allocation error, it may crash.\");\n"
			"    }\n"
			"    show_error = false;\n"
			"    ac_heap_ptr = base;\n"
			"    set_int(0, -1);\n"
			"  }\n"
			"  else {\n"
			"    set_int(0, base);\n"
			"  }\n"
			"\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::lseek()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"lseek\");\n"
			"  int fd = get_int(0);\n"
			"  int offset = get_int(1);\n"
			"  int whence = get_int(2);\n"
			"  int ret = ::lseek(fd, offset, whence);\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::fstat()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"fstat\");\n"
			"  static bool fstat_warn = true;\n"
			"  if (fstat_warn) {\n"
			"    AC_WARN(\"This version of fstat should not be called!\");\n"
			"    AC_WARN(\"Please, recompile your target application with an updated libac_sysc.\");\n"
			"    fstat_warn = false;\n"
			"  }\n"
			"  int fd = get_int(0);\n"
			"  struct stat buf;\n"
			"  int ret = ::fstat(fd, &buf);\n"
			"  if (ret == -1) {\n"
			"    AC_RUN_ERROR(\"System Call fstat (fd %%d): %%s\\n\", fd, strerror(errno));\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::_exit()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"_exit\");\n"
			"  int ac_exit_status = get_int(0);\n"
			"#ifdef USE_GDB\n"
			"  if (gdbstub) gdbstub->exit(ac_exit_status);\n"
			"#endif /* USE_GDB */\n"
			"  stop(ac_exit_status);  \n"
			"}\n"
			"\n"
			"\n"
			"void %s::times()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"times\");\n"
			"  unsigned char zeros[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};\n"
			"  set_buffer(0, zeros, 16);\n"
			"  set_int(0, 0);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::time()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"time\");\n"
			"  int t = get_int(0);\n"
			"  int ret = ::time(0);\n"
			"  if (t!=0) set_buffer(0, (unsigned char *) &ret, 4);\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::random()\n"
			"{\n"
			"  DEBUG_SYSCALL(\"random\");\n"
			"  int ret = ::random();\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"#include <ac_syscall_codes.h>\n"
			"\n"
			"void %s::ac_syscall_wrapper()\n"
			"{\n"
			"  int ret = -1;\n"
			"  unsigned char pathname[100];\n"
			"  int mode;\n"
			"  int fd, newfd;\n"
			"  static struct stat buf_stat;\n"
			"  //static struct tms  buf_tms;\n"
			"\n"
			"  int syscall_code = get_int(0);\n"
			"\n"
			"  switch(syscall_code) {\n"
			"\n"
			"  case __NR_getpid:\n"
			"    DEBUG_SYSCALL(\"getpid\");\n"
			"    ret = 123;\n"
			"    break;\n"
			"\n"
			"  case __NR_chmod:\n"
			"    DEBUG_SYSCALL(\"chmod\");\n"
			"    get_buffer(0, pathname, 100);\n"
			"    mode = get_int(1);\n"
			"    ret = ::chmod((char*)pathname, mode);\n"
			"    break;\n"
			"\n"
			"  case __NR_dup:\n"
			"    DEBUG_SYSCALL(\"dup\");\n"
			"    fd = get_int(1);\n"
			"    ret = ::dup(fd);\n"
			"    break;\n"
			"\n"
			"  case __NR_dup2:\n"
			"    DEBUG_SYSCALL(\"dup2\");\n"
			"    fd = get_int(1);\n"
			"    newfd = get_int(2);\n"
			"    ret = ::dup2(fd, newfd);\n"
			"    break;\n"
			"\n"
			"  case __NR_fstat:\n"
			"    DEBUG_SYSCALL(\"fstat\");\n"
			"    fd = get_int(1);\n"
			"    ret = ::fstat(fd, &buf_stat);\n"
			"    break;\n"
			"\n"
			"\n"
			"    /* Special cases for the fields of the \"struct stat\":\n"
			"       to convert from glibc to newlib */\n"
			"\n"
			"#define FILL_STRUCT_STAT(x) \\\n"
			"  case __AC_struct_stat_##x: \\\n"
			"    DEBUG_SYSCALL(\"filling struct stat field: \" #x); \\\n"
			"    ret = buf_stat.x; \\\n"
			"    break\n"
			"\n"
			"    FILL_STRUCT_STAT(st_dev);\n"
			"    FILL_STRUCT_STAT(st_ino);\n"
			"    FILL_STRUCT_STAT(st_mode);\n"
			"    FILL_STRUCT_STAT(st_nlink);\n"
			"    FILL_STRUCT_STAT(st_uid);\n"
			"    FILL_STRUCT_STAT(st_gid);\n"
			"    FILL_STRUCT_STAT(st_rdev);\n"
			"    FILL_STRUCT_STAT(st_size);\n"
			"    FILL_STRUCT_STAT(st_blksize);\n"
			"    FILL_STRUCT_STAT(st_blocks);\n"
			"    FILL_STRUCT_STAT(st_atime);\n"
			"    FILL_STRUCT_STAT(st_mtime);\n"
			"    FILL_STRUCT_STAT(st_ctime);\n"
			"\n"
			"#undef FILL_STRUCT_STAT\n"
			"\n"
			"\n"
			"  default:\n"
			"    AC_RUN_ERROR(\"System Call code %%d not implemented yet.\\n\", syscall_code);\n"
			"    exit(EXIT_FAILURE);\n"
			"  }\n"
			"    \n"
			"  //if (ret == -1)   AC_RUN_ERROR(\"System call %%d returned -1.\\n\", syscall_code);\n"
			"  set_int(0, ret);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::ac_syscall_geterrno()\n"
			"{\n"
			"  set_int(0, errno);\n"
			"  return_from_syscall();\n"
			"}\n"
			"\n"
			"\n"
			"void %s::ac_syscall_stat_mode()\n"
			"{\n"
			"  AC_RUN_ERROR(\"System Call ac_syscall_stat_mode not implemented yet.\\n\");\n"
			"  exit(EXIT_FAILURE);\n"
			"}\n"
			"\n"
			"\n", project_name,project_name,project_name,project_name,project_name,project_name,project_name,project_name,project_name,project_name,project_name,
			project_name,project_name,project_name,project_name,project_name);

#endif

/*
	fprintf(output, "//! Functions of decoder module\n//-------------------------------------\n\n");
	fprintf(output, "int %s::ExpandInstrBuffer(int index){\n",project_name);
	fprintf(output, "	//Expand the instruction buffer word by word, the number necessary to read position index\n"
			"	int read = (index + 1) - this->quant;\n"
			"	for(int i=0; i<read; i++){\n"
			"		this->buffer[this->quant + i] = (this->IM)->read(this->decode_pc + (this->quant + i) * sizeof(ac_word));\n"
			"	}\n"
			"	this->quant += read;\n"
			"	return this->quant;\n"
			"}\n\n");
	fprintf(output, "unsigned long long %s::GetBits(unsigned char* bu, int* quant, int last, int quantity, int sign) {\n", project_name);
	fprintf(output, "	ac_word* buffer = (ac_word*) bu;\n"
			"	return 1;\n"
			"}\n\n");      
*/
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
  extern char *project_name;
  FILE *output;

  output = fopen("ac_storage.H", "w");

  print_comment( output, "ArchC Storage header file.");

//  fprintf(output, "using namespace ac_parms;\n\n", project_name);

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
          "#include \"%s_parms.H\"\n"
          //"#include \"archc.H\"\n"
          "\n"
          "//#define AC_STORAGE_VERIFY\n"
          "\n"
          "#ifdef AC_STORAGE_VERIFY\n"
	  "//#include \"archc.H\"\n"
          "#endif\n"
          "\n", project_name);

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
          "      fprintf(stderr,\"Storage %%s: Unable to open binary memory copy from file %%s\", name, progmem);\n"
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
          "      fprintf(stderr, \"Storage %%s: trying to load a file (%%d bytes) bigger then storage size (%%d bytes)\", name, fileLen, size);\n"
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
	  "//! The members C1 and C2 was necessary for adapt ac_regbank from 1.6 to 2.0.\n"
	  "//! But neither C1 nor C2 is used\n"
          "template <std::size_t SIZE, class C1, class C2>\n"
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
  extern char *project_name;

  print_comment( output, "ArchC Storage implementation file.");

  fprintf(output,
          "#include <errno.h>\n"
          "#include \"ac_storage.H\"\n"
//          "#include \"archc.H\"\n"
          "#include \"%s_parms.H\"\n"
          "#include \"ac_utils.H\""
	  "\n""\n", project_name);

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
void fast_CreateFileResourcesHeader()
{
  extern ac_sto_list *storage_list;
  extern int HaveMemHier, HaveFormattedRegs;
  ac_sto_list *pstorage;

  extern char *project_name;

  FILE *output;
  output = fopen("ac_resources.H", "w");

  print_comment( output, "ArchC Resources header file.");

  fprintf( output, "#ifndef  _AC_RESOURCES_H\n");
  fprintf( output, "#define  _AC_RESOURCES_H\n\n");

  fprintf( output, "#include  \"%s_parms.H\"\n\n", project_name);
  fprintf( output, "#include  \"ac_storage.H\"\n");
  //fprintf( output, "#include  \"ac_reg.H\"\n\n");

  if( HaveMemHier ){
    fprintf( output, "#include  \"ac_mem.H\"\n");
    fprintf( output, "#include  \"ac_cache.H\"\n");
  }

  fprintf( output, "#include \"ac_reg.H\"\n");

  if( HaveFormattedRegs )
    fprintf( output, "#include  \"ac_fmt_regs.H\"\n");
  fprintf( output, " \n");

  fprintf( output, "\n// extern double& time_step;\n\n");


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
	switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%sextern ac_reg<ac_word> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%sextern ac_reg<bool> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%sextern ac_reg<unsigned char> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%sextern ac_reg<unsigned short> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%sextern ac_reg<unsigned> %s;\n", INDENT[1], pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%sextern ac_reg<unsigned long long> %s;\n", INDENT[1], pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
     }
      break;
                        
    case REGBANK:
      fprintf( output, "%sextern ac_regbank<%u, ac_word, ac_Dword> %s;\n", INDENT[1], pstorage->size, pstorage->name);
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
  fprintf( output, "// extern reg_uint ac_pc;\n");
  fprintf( output, "  extern ac_reg<unsigned> ac_pc;\n");

  //fprintf( output, "  extern reg_word_t ac_branch;\n");
  fprintf( output, "  extern unsigned long long ac_instr_counter;\n");
  fprintf( output, "  extern bool ac_wait_sig;\n");
  fprintf( output, "  extern bool ac_tgt_endian;\n");
//  fprintf( output, "  extern unsigned ac_start_addr;\n\n");

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
void fast_CreateFileResourcesImpl()
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
	switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%sac_reg<ac_word> %s(\"%s\", 0);\n", INDENT[1],  pstorage->name, pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%sac_reg<bool> %s(\"%s\", 0);\n", INDENT[1], pstorage->name, pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%sac_reg<unsigned char> %s(\"%s\", 0);\n", INDENT[1], pstorage->name, pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%sac_reg<unsigned short> %s(\"%s\", 0);\n", INDENT[1], pstorage->name, pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%sac_reg<unsigned> %s(\"%s\", 0);\n", INDENT[1], pstorage->name, pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%sac_reg<unsigned long long> %s(\"%s\", 0);\n", INDENT[1], pstorage->name, pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
      }
      break;
                        
    case REGBANK:
      fprintf( output, "%sac_regbank<%u, ac_word, ac_Dword> %s(\"%s\", %d);\n", INDENT[1], pstorage->size, pstorage->name, pstorage->name, pstorage->size);
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
  fprintf( output, "\n");
  fprintf( output, "//  reg_uint ac_pc = %d;\n", prog_entry_point);
  fprintf( output, "  ac_reg<unsigned> ac_pc (\"ac_pc\", %d);\n", prog_entry_point);
  
//fprintf( output, "  reg_word_t ac_branch = 0;\n");
  fprintf( output, "  unsigned long long ac_instr_counter = 0;\n");
  fprintf( output, "  bool ac_wait_sig;\n");
  fprintf( output, "  bool ac_tgt_endian = %d;\n", ac_tgt_endian);
//  fprintf( output, "  unsigned ac_start_addr = 0;\n");

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





/*!Create ArchC Resources Header File */
/*!Use structures built by the parser.*/
void fast_CreateResourcesHeader(FILE *stream)
{
  extern ac_sto_list *storage_list;
  extern int HaveMemHier, HaveFormattedRegs;
  ac_sto_list *pstorage;

  extern char *project_name;

  FILE *output  = stream;

  /* Declaring storage devices */
  COMMENT(INDENT[6],"Storage Devices.");
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

    switch( pstorage->type ){

    case REG:
      //Formatted registers have a special class.
      if( pstorage->format != NULL ){
        fprintf( output, "%sac_%s %s;\n", INDENT[6], pstorage->name, pstorage->name);
      }
      else{
	switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%sac_reg<ac_word> %s;\n", INDENT[6], pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%sac_reg<bool> %s;\n", INDENT[6], pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%sac_reg<unsigned char> %s;\n", INDENT[6], pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%sac_reg<unsigned short> %s;\n", INDENT[6], pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%sac_reg<unsigned> %s;\n", INDENT[6], pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%sac_reg<unsigned long long> %s;\n", INDENT[6], pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
     }
      break;
                        
    case REGBANK:
	//if (ACMulticoreFlag)
	      fprintf( output, "%sac_regbank<%u, ac_word, ac_Dword> %s;\n", INDENT[6], pstorage->size, pstorage->name);
      break;

    case CACHE:
    case ICACHE:
    case DCACHE:

      if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%sac_storage %s;\n", INDENT[6], pstorage->name);
      }
      else{
        //It is an ac_cache object.
        fprintf( output, "%sac_cache %s;\n", INDENT[6], pstorage->name);
      }

      break;

    case MEM:

      if( !HaveMemHier ) { //It is a generic mem. Just emit a base container object.
        fprintf( output, "%sac_storage %s;\n", INDENT[6], pstorage->name);
      }
      else{
        //It is an ac_mem object.
        fprintf( output, "%sac_mem %s;\n", INDENT[6], pstorage->name);
      }

      break;

      
    default:
      fprintf( output, "%sac_storage %s;\n", INDENT[6], pstorage->name);      
      break;
    }
  }

  fprintf( output, "\n");

  COMMENT(INDENT[6],"Control Variables.","\n");
//  fprintf( output, "// extern reg_uint ac_pc;\n");
  fprintf( output, "%sac_reg<unsigned> ac_pc;\n", INDENT[6]);

  //fprintf( output, "  extern reg_word_t ac_branch;\n");
  //if (ACMulticoreFlag)
	fprintf( output, "%sunsigned long long ac_instr_counter;\n", INDENT[6]);
  
  fprintf( output, "%sbool ac_wait_sig;\n", INDENT[6]);
  fprintf( output, "%sbool ac_tgt_endian;\n", INDENT[6]);
//  fprintf( output, "%sunsigned ac_start_addr;\n\n", INDENT[6]);

}


/*!Create ArchC Resources Implementation File */
/*!Use structures built by the parser.*/
void fast_CreateResourcesImpl(FILE *stream)
{
  extern ac_sto_list *storage_list;
  ac_sto_list *pstorage;
  extern int HaveMemHier;
  extern int ac_tgt_endian;

  int i;

  FILE *output = stream;
  fprintf( output, "\n");

//  COMMENT(INDENT[6],"Storage Devices.");
  for( pstorage = storage_list; pstorage != NULL; pstorage=pstorage->next){

    switch( pstorage->type ){

    case REG:

      //Formatted registers have a special class.
      if( pstorage->format != NULL ){
        fprintf( output, "%s%s(\"%s\"),\n", INDENT[6], pstorage->name, pstorage->name);
      }
      else{
	switch( (unsigned)(pstorage->width) ){
	    case 0:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6],  pstorage->name, pstorage->name);
	      break;
	    case 1:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6], pstorage->name, pstorage->name);
	      break;
	    case 8:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6], pstorage->name, pstorage->name);
	      break;
	    case 16:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6], pstorage->name, pstorage->name);
	      break;
	    case 32:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6], pstorage->name, pstorage->name);
	      break;
	    case 64:
	      fprintf( output, "%s%s(\"%s\", 0),\n", INDENT[6], pstorage->name, pstorage->name);
	      break;
	    default:
	      AC_ERROR("Register width not supported: %d\n", pstorage->width);
	      break;
	  }
      }
      break;
                        
    case REGBANK:
     // if (ACMulticoreFlag)
	      fprintf( output, "%s%s(\"%s\", %d),\n", INDENT[6], pstorage->name, pstorage->name, pstorage->size);
      break;

    case CACHE:
    case ICACHE:
    case DCACHE:

      if( !pstorage->parms ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%s%s(\"%s\", %d),\n", INDENT[6], pstorage->name, pstorage->name, pstorage->size);
      }
      else{
        //It is an ac_cache object.
        EmitCacheDeclaration(output, pstorage, 1);
      }
      break;

    case MEM:

      if( !HaveMemHier ) { //It is a generic cache. Just emit a base container object.
        fprintf( output, "%s%s(\"%s\", %d),\n", INDENT[6], pstorage->name, pstorage->name, pstorage->size);
      }
      else{
        //It is an ac_mem object.
        fprintf( output, "%s%s(\"%s\", %d),\n", INDENT[6], pstorage->name, pstorage->name, pstorage->size);
      }
      break;

    default:
      fprintf( output, "%s%s(\"%s\", %d),\n", INDENT[6], pstorage->name, pstorage->name, pstorage->size);
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

//  fprintf( output, "\n");

//  COMMENT(INDENT[1],"Control Variables","\n");
//  fprintf( output, "//  reg_uint ac_pc = %d;\n", prog_entry_point);
  fprintf( output, "%sac_pc (\"ac_pc\", %d),\n", INDENT[6], prog_entry_point); 
//  fprintf( output, "%sac_instr_counter (0),\n", INDENT[6]);
  fprintf( output, "%sac_wait_sig (0),\n", INDENT[6]);
  fprintf( output, "%sac_tgt_endian (%d),\n", INDENT[6], ac_tgt_endian);
//  fprintf( output, "%sac_start_addr (0),\n", INDENT[6]);

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
  //extern char *ACVERSION;
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

  if (!ACMulticoreFlag)
	fprintf(output, "#include \"ac_resources.H\"\n");
  
  fprintf(output,
          //"#include archc.H\"\n"
          //"#include \"archc.cpp\"\n"
          //"#include \"%s_isa.H\"\n"
          "#include \"ac_progmem.H\"\n"
	  "#include \"%s.H\"\n"
          "#include <iostream>\n"
          "#include <systemc.h>\n"
	  "\nusing namespace std;"
	  "\n"
          "\n"
          , project_name, project_name);

  fprintf( output, "const char *project_name=\"%s\";\n", project_name);
  fprintf( output, "const char *project_file=\"%s\";\n", arch_filename);
  fprintf( output, "const char *archc_version=\"%s\";\n", ACVERSION);
  fprintf( output, "const char *archc_options=\"%s\";\n", ACOptions);
  fprintf( output, "\n");
  
//  fprintf( output, "unsigned int ac_heap_ptr;\n");
//  fprintf( output, "char *appfilename;\n");
//  fprintf( output, "unsigned ac_start_addr = 0;\n\n");
//  fprintf( output, "int ac_exit_status;\n") ;
//  fprintf( output, "int ac_stop_flag;\n\n"); 

  if (ACStatsFlag) {
    fprintf(output, "#include \"ac_stats.H\"\n");
    fprintf(output, "ac_stats ac_sim_stats;\n\n");
  }

  fprintf(output,
          "int sc_main(int ac, char *av[])\n"
          "{\n"
          "  %s %s_proc1;\n\n", project_name, project_name);
         
  fprintf(output, 
	  "  %s_proc1.appfilename=\"%s\";\n"
          "\n"
          "  %s_proc1.ac_heap_ptr = 0x%x;\n"
          "  %s_proc1.ac_start_addr = 0x%x;\n"
          "\n",  project_name, ACCompsimProg, project_name, ac_heap_ptr, project_name, ac_start_addr);
  
  if (ACMulticoreFlag)
  	  fprintf(output, "  %s_proc1.%s.load_array(mem_dump, 0x%x);\n", project_name, (accs_FindLoadDevice())->name, ac_heap_ptr);
  else
  	  fprintf(output, "  %s.load_array(mem_dump, 0x%x);\n", (accs_FindLoadDevice())->name, ac_heap_ptr);

  fprintf(output,           "\n"
	  "#ifdef AC_DEBUG\n"
          "  ac_trace(\"%s.trace\");\n"
          "#endif\n"
          "\n"
          "  %s_proc1.init(ac, av);\n"
          "\n" 
          "  %s_proc1.start();\n"
          "\n"
          "  std::cerr << std::endl;\n"
          "  %s_proc1.PrintStat();\n"
          "  std::cerr << std::endl;\n"
          "\n"
          //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
          ,project_name, project_name, project_name, project_name);

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
          "  return %s_proc1.ac_exit_status;\n"
          "};\n", project_name
          );

  fclose( output); 
}


/*!Create ArchC ISA Header File */
/*!Use structures built by the parser.*/
void accs_CreateISAHeader()
{
  extern char *project_name;
  extern ac_decoder_full *decoder;
  extern ac_dec_field *common_instr_field_list;

  ac_dec_field *pfield;
  ac_dec_format *pformat;
  ac_dec_instr *pinstr;



  FILE *output;
  char filename[50];
  
  snprintf(filename, 50, "%s_isa.H", project_name);
  output = fopen(filename, "w");

  print_comment( output, "ISA header file.");

  fprintf(output,
          "#ifndef _ISA_H\n"
          "#define _ISA_H\n"
          "\n"
          //"#include \"ac_resources.H\"\n"
          "#include \"%s_parms.H\"\n"
          //"#include \"debug.h\"\n"
          //"#include <stdio.h>\n"
          //"#include <systemc.h>\n"
          "\n"
          "//Auxiliary: SIGNED\n"
          "#define SIGNED(val,size) \\\n"
          "     ( (val >= (1 << (size-1))) ? val | (0xFFFFFFFF << size) : val )\n"
          "\n", project_name
          );

  //Begin function
  COMMENT(INDENT0,"Begin function","\n");
  fprintf( output,
           "//namespace ac_begin {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0);}\n"
           "//#define AC_ARGS_begin (ac_stage_list a, unsigned b)\n"
           "#define AC_BEHAVIOR_begin() %s::ac_behavior_begin()\n"
           "\n", project_name
           );

  //End function
  COMMENT(INDENT0,"End function","\n");
  fprintf( output,
           "//namespace ac_end {void behavior(ac_stage_list a=ac_stage_list(0), unsigned b=0);}\n"
           "//#define AC_ARGS_end (ac_stage_list a, unsigned b)\n"
           "#define AC_BEHAVIOR_end() %s::ac_behavior_end()\n"
           "\n", project_name
           );

  //Generic instruction behavior
/*
  COMMENT(INDENT0,"Generic instruction behavior","\n");
  fprintf( output,
           "#define AC_ARGS_instruction (unsigned ac_instr_size)\n"
           "#define AC_BEHAVIOR_instruction() %s::ac_behavior_instruction AC_ARGS_instruction\n"
           "\n", project_name
           );
*/

  COMMENT(INDENT0,"Generic instruction behavior","\n");
  fprintf( output,
           "#define AC_ARGS_instruction (unsigned ac_instr_size " );

  /* common_instr_field_list has the list of fields for the generic instruction. */
  for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
      fprintf(output, ", ");
      if( pfield->sign )
        fprintf(output, "int %s", pfield->name);
      else 
        fprintf(output, "unsigned int %s", pfield->name);
    }    
    fprintf(output, ")\n\n");


   fprintf( output, "#define AC_BEHAVIOR_instruction() %s::ac_behavior_instruction AC_ARGS_instruction\n", project_name);


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
    fprintf( output, "#define AC_BEHAVIOR_%s() %s::ac_behavior_%s AC_ARGS_%s\n",
             //cygwin don't recognize %1$s to specify an argument 'cause uses newlib
             pformat->name, project_name, pformat->name, pformat->name);
  }
  fprintf( output, "\n");

  //Macros for instruction behaviors
  COMMENT(INDENT0,"Macros for instruction behaviors","\n");
  fprintf( output, "#define ac_behavior(instr)   AC_BEHAVIOR_##instr ()\n\n");
  for( pinstr = decoder->instructions; pinstr!= NULL; pinstr=pinstr->next) {
    fprintf( output, "#define AC_BEHAVIOR_%s() %s::ac_behavior_%s AC_ARGS_%s\n",
             pinstr->name, project_name, pinstr->name, pinstr->format);
  }
  fprintf( output, "\n");

#if 0 
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
#endif  
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

char *strtoupper(char *str){
  char *string = (char *) malloc (sizeof(char) * 50);
  int i;
  
  strcpy(string, str);

  if(string){
    for(i = 0; string[i]!='\0';i++)
      string[i] = toupper(string[i]);
  }

  return string;
}

/*!Create ArchC Architecture Dependent Syscalls Header File */
/*!Use structures built by the parser.*/
void accs_CreateArchSyscallHeader()
{
  extern char *project_name;
  FILE *output;
  char filename[50];
  
  snprintf(filename, 50, "%s_syscall_macros.H", project_name);
  output = fopen(filename, "w");
  fprintf( output, "//In Compiled Simulation, no exist model_syscall class. That methods in model class.\n\n");
  fprintf( output, "#ifndef %s_SYSCALL_H\n", project_name);
  fprintf( output, "#define %s_SYSCALL_H\n", project_name);
  fprintf( output, "#include \"%s_parms.H\"\n", project_name);
  fprintf( output, "#include \"%s.H\"\n", project_name);
  fprintf( output, "#define %s_syscall %s\n\n", project_name, project_name);
  fprintf( output, "#endif \n");

  fclose( output);

  snprintf(filename, 50, "%s_syscall.H.tmpl", project_name);
  output = fopen(filename, "w");
  fprintf( output, "//In Compiled Simulation, this file is Empty\n\n");
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
                  "      "#NAME "();\n" \
                  "      ac_instr_counter++;\n"); \
	  if (ACStatsFlag) fprintf(output, "      ac_sim_stats.syscall_executed++;\n"); \
          if (strcmp( #NAME , "_exit")==0) fprintf( output, "     return;\n"); \
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
   extern ac_dec_field *common_instr_field_list;
    ac_dec_field *pfield;

   char *args_copy = (char *) malloc (sizeof(char)*500);
   char *field_common = (char*) malloc (sizeof(char)*100);
   char *tokenPtr;

   *field_common = '\0';
   strcpy(args_copy, args);

//Print instruction behavior hierarchy to output
//  fprintf( output, "%*sac_behavior_instruction(%d);\n", indent, " ", pinstr->size * 8); /* ac_instr_size: in bits */
  
  fprintf( output, "%*sac_behavior_instruction(%d ", indent, " ", pinstr->size * 8); /* ac_instr_size: in bits */
  for( pfield = common_instr_field_list; pfield != NULL; pfield = pfield->next){
//      fprintf(output, ", %d", *decode_table[j]->dec_vector);
	  strcat (field_common, ", ");
	  tokenPtr = strtok(args_copy, ",");
	  strcat (field_common, tokenPtr);
    }
  fprintf(output, "%s);\n", field_common);

  if (ACAnnulSigFlag)
	  fprintf(output, "%*sif (!ac_annul_sig) { \n", indent, " ");

  fprintf( output, "%*sac_behavior_%s(%d, %s);\n", indent, " ", pinstr->format, pinstr->size * 8, args);
  fprintf( output, "%*sac_behavior_%s(%d, %s);\n", indent, " ", pinstr->name, pinstr->size * 8, args);

  if (ACAnnulSigFlag)
  {
//	  fprintf( output, "%*sac_instr_counter--;\n", indent, " ");
	  fprintf( output, "%*s} else\n", indent, " ");
	  fprintf( output, "%*sac_annul_sig = 0;\n",indent, " ");
  }
}


void accs_EmitInstrExtraTop(FILE* output, int j, ac_dec_instr *pinstr, int indent)
{
  //Print extra instruction actions before behavior to output
  fprintf( output, "%*sPRINT_TRACE;\n", indent, " ");
}


void accs_EmitInstrExtraBottom(FILE* output, int j, ac_dec_instr *pinstr, int indent)
{
  //MAXIWELL
  //Print extra instruction actions after behavior to output
  if ((ACMulticoreFlag==0)  ||   ((ACMulticoreFlag==1)&&(PROCESSOR_OPTIMIZATIONS == 0)))
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
  if (PROCESSOR_OPTIMIZATIONS==2)
  	fprintf( output, "CFLAGS := $(CFLAGS) $(if $(filter 1,$(INLINE)),-O3 -finline-functions -fgcse) $(if $(filter 1,$(ISA_AND_SYSCALL_TOGETHER)), -DAC_INLINE) ");
  else	
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

  /* If there is only one level, which is gonna be zero, then it is the same */
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


