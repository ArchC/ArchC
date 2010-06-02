/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      version_definitions.H
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
 * @brief     Defined symbol versions table information management.
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _VERSION_DEFINITIONS_H
#define _VERSION_DEFINITIONS_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

namespace ac_dynlink {

  /* Manages versions defined by a DSO (dynamically shared object) */
  class version_definitions {
  private:
    bool match_endian;
    unsigned char *strtab;
    unsigned char *ver_def;
    Elf32_Verdef * current_entry;
  public:
    version_definitions(unsigned char *strtab, unsigned char *ver_def, bool match_endian);
    ~version_definitions();

    bool set_entry(char *vername, Elf32_Word hash);
    bool set_entry(Elf32_Half index);
    Elf32_Word get_cur_hash();
    Elf32_Half get_cur_flags();
    char * get_cur_name();
  };


}

#endif
