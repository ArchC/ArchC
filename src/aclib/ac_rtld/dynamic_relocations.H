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
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
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
