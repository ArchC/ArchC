/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      memmap.H
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
 * @brief     Memory map handling classes
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _MEMMAP_H
#define _MEMMAP_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

namespace ac_dynlink {

 enum memmap_status {MS_FREE, MS_USED};
  
  /* This class is a linked list node of contiguous regions
     of memory, to indicate whether they are allocated for a
     shared library or not. */
  class memmap_node {
  private:
    memmap_node *next;
    Elf32_Addr addr;
    memmap_status status;
  public:
    memmap_node(memmap_node * _next, memmap_status _status, Elf32_Addr _addr);
    
    memmap_node *get_next();
    
    Elf32_Addr get_addr();
    
    memmap_status get_status();

    void set_status(memmap_status value);

    void set_next(memmap_node *next);
  };
    
  /* This class manages a memory map */
  class memmap {
  private:
    memmap_node *list;
    long pagesize;
    Elf32_Addr memsize;
    Elf32_Addr brkaddr;
    Elf32_Addr newbrkaddr;
    bool warning_display;
  protected:
    void delete_node(memmap_node *node); 

    void fix_consistency();  

    void free_memmap(); 
  public:
    memmap();

    ~memmap();

    void set_memsize(Elf32_Addr memsize);

    void set_brk_addr(Elf32_Addr addr);

    bool verify_region_availability(Elf32_Addr addr, Elf32_Word size,
                                    Elf32_Addr *nextaddr);

    memmap_node *find_region (Elf32_Addr addr);

    memmap_node *add_region (Elf32_Addr start_addr, Elf32_Word size); 

    Elf32_Addr suggest_free_region (Elf32_Word size); 

    Elf32_Addr suggest_mmap_region(Elf32_Word size);

    Elf32_Addr brk(Elf32_Addr addr);

    bool munmap(Elf32_Addr addr, Elf32_Word size);

    Elf32_Addr mmap_anon(Elf32_Addr addr, Elf32_Word size);

  };

}

#endif
