/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_relocations.cpp
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
 * @brief     ArchC classes representing ELF dynamic relocations structures (implementation)
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

#include <stdlib.h>

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include <ac_utils.H>
#include "dynamic_relocations.H"

namespace ac_dynlink {

  /* class dynamic_relocations */

  /* Public methods */
  dynamic_relocations::dynamic_relocations() {
    rel = NULL;
    rela = NULL;
    use = AC_NO_RELOC;
  }

  void dynamic_relocations::setup(Elf32_Addr addr, Elf32_Word size, unsigned char *mem, 
				  unsigned int use, bool match_endian) {
    this->match_endian = match_endian;
    this->use = use;
    this->size = size;
    if (use == AC_USE_REL) {
      rel = reinterpret_cast<Elf32_Rel *>(mem + addr);
    } else if (use == AC_USE_RELA) {
      rela = reinterpret_cast<Elf32_Rela *>(mem + addr);
    }
  }

  Elf32_Word dynamic_relocations::get_size() {
    return size;
  }

  unsigned int dynamic_relocations::get_use() {
    return use;
  }
  
  Elf32_Addr dynamic_relocations::read_offset (unsigned int ndx) {
    if (ndx >= size)
      return 0;
    if (use == AC_USE_REL)
      return convert_endian(4, rel[ndx].r_offset, match_endian);
    else if (use == AC_USE_RELA)
      return convert_endian(4, rela[ndx].r_offset, match_endian);
    return 0;
  }

  Elf32_Word dynamic_relocations::read_info (unsigned int ndx) {
    if (ndx >= size)
      return 0;
    if (use == AC_USE_REL)
      return convert_endian(4, rel[ndx].r_info, match_endian);
    else if (use == AC_USE_RELA)
      return convert_endian(4, rela[ndx].r_info, match_endian);
    return 0;
  }
  
  Elf32_Sword dynamic_relocations::read_addend (unsigned int ndx) {
    if (use != AC_USE_RELA || ndx >= size)
      return 0;
    return convert_endian(4, rela[ndx].r_addend, match_endian);
  }

}
