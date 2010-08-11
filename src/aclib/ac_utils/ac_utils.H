/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_utils.H
 * @author    Sandro Rigo
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     Include file for ArchC library
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef _ARCHC_H
#define _ARCHC_H

#include <systemc.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include <list>
#include <iomanip>
#include <iostream>
#include <fstream>

// Forward declarations of ac_arch and ac_arch_ref.
template<class ac_word, class ac_Hword> class ac_arch;
template<class ac_word, class ac_Hword> class ac_arch_ref;

using std::list;
using std::setw;

extern int ac_argc;
extern char **ac_argv;

// Prototypes
void ac_init_opt( int ac, char* av[]);
void ac_init_app( int ac, char* av[]);


//////////////////////////////////////////
// ArchC Defines                        //
//////////////////////////////////////////

#define ac_err stderr
#define IDENT         0


#define AC_ERROR( msg )    std::cerr<< "ArchC ERROR: " << msg  <<'\n'
#define AC_WARN( msg )     std::cerr<< "ArchC Warning: " << msg  <<'\n'
#define AC_SAY( msg )      std::cerr<< "ArchC: " << msg  <<'\n'

#ifndef AC_COMPSIM
#define AC_RUN_ERROR( str, args...) \
  fprintf(stderr, "ArchC Runtime error (ac_pc=%#x; ac_instr_counter=%llu): " str, get_ac_pc(), ac_instr_counter , ##args)
#else //AC_COMPSIM
#define AC_RUN_ERROR( str, args...) \
  fprintf(stderr, "ArchC Runtime error (ac_pc=%#x; ac_instr_counter=%llu): " str, (int) ac_pc, ac_instr_counter , ##args)
#endif

#define AC_RUN_MSG( str )  fprintf(stderr, str);

#define ac_trace( f )      extern ofstream trace_file; extern bool ac_do_trace; ac_do_trace = 1; trace_file.open( f )
#define ac_close_trace     trace_file.close


#ifdef AC_COMPSIM

//FIXME: 
//Signal handlers for interrupt and segmentation fault (set in ac_start() )
#include <signal.h>
typedef void (*sighandler_t)(int);
void sigint_handler(int signal)
{
  fprintf(stderr, "ArchC: INTERUPTED BY THE SIGNAL %d\n", signal);
//  PrintStat();
  exit(EXIT_FAILURE);
}
void sigsegv_handler(int signal)
{
  fprintf(stderr, "ArchC Error: Segmentation fault.\n");
//  PrintStat();
  exit(EXIT_FAILURE);
}
void sigusr1_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Printing statistics\n", signal);
//  PrintStat();
  fprintf(stderr, "ArchC: -------------------- Continuing Simulation ------------------\n");
}

#ifdef USE_GDB
void sigusr2_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Starting GDB support\n", signal);
  gdbstub->enable();
  gdbstub->connect();
}
#endif /* USE_GDB */


#endif //AC_COMPSIM


// byte_swap functions, with special assembly-language optimizations for the
// i486 or greater processors.
#if __i486__ && __i586__ && __i686__
inline uint16_t byte_swap(uint16_t value) {
  asm ("rol $8, %0"
       : "=r" (value)
       : "r"  (value)
      );
  return value;
}

inline uint32_t byte_swap(uint32_t value) {
  asm ("bswap %0"
       : "=r" (value)
       : "r"  (value)
      );
  return value;
}
#endif // __i486__ && __i586__ && __i686__

template <typename T>
inline T byte_swap(T value) {
  unsigned int aux;
  T result = 0;
  for (unsigned int i = 0; i < sizeof(T); i++) {
    aux = value & 0xff;
    value  >>= 8;
    result <<= 8;
    result |=  aux;
  }
  return result;
}

// convert_endian is redundant, the idea is to use byte_swap for all
// endianness conversions in the future.
unsigned int convert_endian(unsigned int size, unsigned int num, bool match_endian);

#ifndef AC_COMPSIM
#include "ac_arch_ref.H"
#endif

//Loading binary application
// int ac_load_elf(char* filename, unsigned char* data_mem, unsigned int data_mem_size)
/// Template wrapper class for memory access. 
template <typename ac_word, typename ac_Hword> int ac_load_elf(ac_arch_ref<ac_word, ac_Hword> &ref, char* filename, unsigned char* data_mem, unsigned int data_mem_size, unsigned int& ac_heap_ptr, unsigned int& ac_start_addr, bool match_endian)
{ 
  Elf32_Ehdr    ehdr;
  Elf32_Shdr    shdr;
  Elf32_Phdr    phdr;
  int           fd;
  unsigned int  i;
  int           is_dyn = 0;           /* If executable has INTERP segment */
  Elf32_Addr    dynamic_address = 0;  /* DYNAMIC segment address (if present) */
  Elf32_Word    size = 0;             /* Total size occupied by the executable file in memory */
  unsigned char pinterp[256];          /* Program interpreter name */

  //Open application
  if (!filename || ((fd = open(filename, 0)) == -1)) {
    AC_ERROR("Opening application file '" << filename << "': " << strerror(errno) << endl);
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
  ac_start_addr = convert_endian(4,ehdr.e_entry, match_endian);
  if (ac_start_addr > data_mem_size) {
    AC_ERROR("the start address of the application is beyond model memory\n");
    close(fd);
    exit(EXIT_FAILURE);
  }

  if (convert_endian(2,ehdr.e_type, match_endian) == ET_EXEC) {
    
    //It is an ELF file
    AC_SAY("Reading ELF application file: " << filename);

    //Get program headers and load segments
    //    lseek(fd, convert_endian(4,ehdr.e_phoff, match_endian), SEEK_SET);
    for (i=0; i<convert_endian(2,ehdr.e_phnum, match_endian); i++) {
      unsigned int segment_type;

      //Get program headers and load segments
      lseek(fd, convert_endian(4,ehdr.e_phoff, match_endian) + convert_endian(2,ehdr.e_phentsize, match_endian) * i, SEEK_SET);
      if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
        AC_ERROR("reading ELF program header\n");
        close(fd);
        exit(EXIT_FAILURE);
      }

      segment_type = convert_endian(4, phdr.p_type, match_endian);
      
      switch(segment_type) {
      case PT_INTERP: { // Requesting program interpreter
        

        Elf32_Off p_offset = convert_endian(4, phdr.p_offset, match_endian);
        Elf32_Word p_filesz = convert_endian(4, phdr.p_filesz, match_endian);

        lseek(fd, p_offset, SEEK_SET);
        if (read(fd, pinterp, (p_filesz > 255)? 255: p_filesz) != (signed)((p_filesz > 255)?255: p_filesz)) {
          AC_ERROR("reading program interpreter segment\n");
          close(fd);
          exit(EXIT_FAILURE);
        }
        pinterp[(p_filesz > 255)? 255 : p_filesz] = 0; /* Terminate string */
        
        is_dyn = 1;
        break;
      }
      case PT_DYNAMIC:  // Dynamic information
        dynamic_address = convert_endian(4, phdr.p_vaddr, match_endian);
        /* Fall through. */
      case PT_LOAD: { // Loadable segment type - load dynamic segments as well
        Elf32_Addr p_vaddr = convert_endian(4,phdr.p_vaddr, match_endian);
        Elf32_Word p_memsz = convert_endian(4,phdr.p_memsz, match_endian);
        Elf32_Word p_filesz = convert_endian(4,phdr.p_filesz, match_endian);
        Elf32_Off  p_offset = convert_endian(4,phdr.p_offset, match_endian);
        
        //Error if segment greater then memory
        if (data_mem_size < p_vaddr + p_memsz) {
          AC_ERROR("not enough memory in ArchC model to load application.\n");
          close(fd);
          exit(EXIT_FAILURE);
        }
        
        //Set heap to the end of the segment
        if (ac_heap_ptr < p_vaddr + p_memsz) ac_heap_ptr = p_vaddr + p_memsz;

        //Update size value
        if (p_vaddr + p_memsz > size)
          size = p_vaddr + p_memsz;

        //Load 
        lseek(fd, p_offset, SEEK_SET);
        if (read(fd, data_mem + p_vaddr, p_filesz) != (signed)p_filesz) {
          AC_ERROR("reading ELF LOAD segment.\n");
          close(fd);
          exit(EXIT_FAILURE);
        }
        memset(data_mem + p_vaddr + p_filesz, 0, p_memsz - p_filesz);
        break;
      }
      default:
        break;
      }

      //next header/segment
      //      lseek(fd, convert_endian(4,ehdr.e_phoff, match_endian) + convert_endian(2,ehdr.e_phentsize, match_endian) * i, SEEK_SET);
    }
  }
  else if (convert_endian(2,ehdr.e_type, match_endian) == ET_REL) {

    AC_SAY("Reading ELF relocatable file: " << filename);

    // first load the section name string table
    char *string_table = NULL;
    int   shoff = convert_endian(4,ehdr.e_shoff, match_endian);
    short shndx = convert_endian(2,ehdr.e_shstrndx, match_endian);
    short shsize = convert_endian(2,ehdr.e_shentsize, match_endian);

    lseek(fd, shoff+(shndx*shsize), SEEK_SET);
    if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
      AC_ERROR("reading ELF section header\n");
      close(fd);
      exit(EXIT_FAILURE);
    }

    string_table = (char *) malloc(convert_endian(4,shdr.sh_size, match_endian));
    lseek(fd, convert_endian(4,shdr.sh_offset, match_endian), SEEK_SET);
    if (read(fd, string_table, convert_endian(4,shdr.sh_size, match_endian)) !=
        (signed)convert_endian(4,shdr.sh_size, match_endian)) {
      AC_ERROR("reading ELF string table.\n");
      close(fd);
      exit(EXIT_FAILURE);
    }
    // load .text, .data and .bss sections
    for (i=0; i<convert_endian(2,ehdr.e_shnum, match_endian); i++) {

      lseek(fd, shoff + shsize*i, SEEK_SET);

      if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
        AC_ERROR("reading ELF section header\n");
        close(fd);
        exit(EXIT_FAILURE);
      }


      if (!strcmp(string_table+convert_endian(4,shdr.sh_name, match_endian), ".text") ||
          !strcmp(string_table+convert_endian(4,shdr.sh_name, match_endian), ".data") ||
          !strcmp(string_table+convert_endian(4,shdr.sh_name, match_endian), ".bss")) {

        //        printf("Section %s:\n", string_table+convert_endian(4,shdr.sh_name, match_endian));

        Elf32_Off  tshoff  = convert_endian(4,shdr.sh_offset, match_endian);
        Elf32_Word tshsize = convert_endian(4,shdr.sh_size, match_endian);
        Elf32_Addr tshaddr = convert_endian(4,shdr.sh_addr, match_endian);

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

        if (!strcmp(string_table+convert_endian(4,shdr.sh_name, match_endian), ".bss")) {
          memset(data_mem + tshaddr, 0, tshsize);
          //continue;
          break; // .bss is supposed to be the last one
        }

        //Load
        lseek(fd, tshoff, SEEK_SET);
        if (read(fd, data_mem + tshaddr, tshsize) != (signed)tshsize) {
          AC_ERROR("reading ELF section.\n");
          close(fd);
          exit(EXIT_FAILURE);
        }
      }

    }
    if (string_table)
      free(string_table);
  }

  ref.ac_dyn_loader.initiate(ac_start_addr, size, data_mem_size, ac_heap_ptr,
                             fd, match_endian);

  //Close file
  close(fd);

  if (is_dyn) {
    if (!dynamic_address) {
      AC_ERROR("ELF executable requests a program interpreter but does not provide a DYNAMIC segment.");
      exit(EXIT_FAILURE);
    } else { // Call the run time loader
      ref.ac_dyn_loader.loadnlink_all(dynamic_address,(char*)pinterp, data_mem, 
                          ac_start_addr, size, sizeof(ac_word), match_endian,
                          data_mem_size, ac_heap_ptr);
    }
  } else {
    if (dynamic_address)
      AC_WARN("This ELF executable has a DYNAMIC segment but does not requests a program interpreter.\
 This segment will be ignored.");
  }

  return EXIT_SUCCESS;
}

#endif //_ARCHC_H
