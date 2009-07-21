/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_relocations.H
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
 * @brief     ArchC classes representing ELF dynamic relocations structures
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */



#ifndef _DYNAMIC_RELOCATIONS_H
#define _DYNAMIC_RELOCATIONS_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */


namespace ac_dynlink {

  enum {AC_NO_RELOC, AC_USE_REL, AC_USE_RELA};

  /* Stores pending relocations on a loaded shared object. */
  class dynamic_relocations {
  private:
    Elf32_Rel *rel;
    Elf32_Rela *rela;
    Elf32_Word size;
    unsigned int use;
    bool match_endian;
  public:
    dynamic_relocations();

    void setup(Elf32_Addr addr, Elf32_Word size, unsigned char *mem, 
	       unsigned int use, bool match_endian);

    Elf32_Word get_size(); 

    unsigned int get_use(); 

    Elf32_Addr read_offset (unsigned int ndx); 

    Elf32_Word read_info (unsigned int ndx); 

    Elf32_Sword read_addend (unsigned int ndx);

  };


}

#endif
