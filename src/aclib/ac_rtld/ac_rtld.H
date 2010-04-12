/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_rtld.H
 * @author    Rafael Auler
 *
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:19 -0300
 *
 * @brief     The ArchC ELF runtime loader.
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

/***************************************************************************
 *                ArchC ELF Runtime Loader & Dynamic Linker                *
 ***************************************************************************
 *  & Capabilities                                                         *
 *   - DSO loading                                                         *
 *   - Memory map management, assigning free space to a new library        *
 *   - Finding libraries using AC_LIBRARY_PATH enviroment variable         *
 *   - Symbol management & binding                                         *
 *   - Dynamic relocation                                                  *
 *   - Symbol version handling                                             *
 *  & Characteristics                                                      *
 *   - Does NOT use lazy binding                                           *
 *  & Limitations                                                          *
 *   - Can't unload a library                                              *
 *   - Poor library finding/matching mechanism                             *
 ***************************************************************************/

#ifndef _AC_RTLD_H
#define _AC_RTLD_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include "memmap.H"
#include "ac_rtld_config.H"

namespace ac_dynlink {

  /* Forward class declarations */
  class link_node;

  /* Class ac_rtld contains methods and data necessary to the execution of the
     ArchC run time dynamic linker. */
  class ac_rtld {
  private:
    ac_rtld_config rtld_config;   /* Loads relocation code translation from target to ArchC's (if needed) */
    link_node * root;             /* "Root" node of the dynamic references digraph */
    unsigned char word_size;      /* Target architecture word size */
    bool initiated;
    bool glibc;

    bool detect_static_glibc(int fd, bool match_endian);

  public:
    memmap mem_map;               /* Linked list of contiguous regions of memory and their state */


    ac_rtld();
    
    ~ac_rtld();

    void load_libraries(unsigned char *mem, Elf32_Word mem_size);

    void link(unsigned char *mem);

    unsigned *get_init_array();
    unsigned *get_fini_array();
    unsigned  get_init_arraysz();
    unsigned  get_fini_arraysz();
    void  set_init_arraysz(unsigned value);
    void  set_fini_arraysz(unsigned value);
    
    bool is_glibc();
    
    void initiate(Elf32_Addr start_addr, Elf32_Word size, Elf32_Word memsize, Elf32_Word brkaddr, int fd, bool match_endian);
    
    void loadnlink_all(Elf32_Addr dynaddr, const char *pinterp, unsigned char *mem, Elf32_Addr& start_addr, Elf32_Word size,
                       unsigned char word_size, bool match_endian, Elf32_Word mem_size,
                       unsigned int& ac_heap_ptr);
    
  };
  
}

#endif
