/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_info.H
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
 * @brief     Stores information about the DYNAMIC segment found in an ELF file. 
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */


#ifndef _DYNAMIC_INFO_H
#define _DYNAMIC_INFO_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

namespace ac_dynlink {

  /* Forward class declarations */
  class memmap;
  class link_node;
  class version_needed;

  /* Environment variable containing libraries search path */
  #define ENV_AC_LIBRARY_PATH "AC_LIBRARY_PATH"

  /* Class is the representation of the dynamic table of a shared object.
     Has methods to read data from the DYNAMIC segment, retrieve requested 
     values of the table, load shared libraries marked as "NEEDED" by this
     table into application memory.

     To accomplish the latter task, has private methods to verify if a library
     is already loaded and to find and open a library based on its soname.
  */
  class dynamic_info {
  private:
    Elf32_Dyn *dynamic_segment;
    Elf32_Word dynamic_size;
    bool match_endian;

  protected:

    int find_library (const char *soname);

  public:

    dynamic_info();

    ~dynamic_info();

    int compare_library_soname (const char *soname1, const char *soname2);

    Elf32_Word get_value(Elf32_Sword tag);

    bool set_value(Elf32_Sword tag, Elf32_Word value);

    void load_dynamic_info (Elf32_Addr addr, unsigned char *mem, bool match_endian);

    void load_needed (memmap *mem_map, unsigned char *mem, link_node * l_node, Elf32_Word mem_size,
                      version_needed *verneed);

    Elf32_Word load_library (Elf32_Addr load_addr, unsigned char *mem, unsigned char *soname,
			     Elf32_Addr& dyn_addr, Elf32_Word& dyn_size, Elf32_Word mem_size);
  };
}


#endif
