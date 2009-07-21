/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      version_needed.H
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
 * @brief     Needed version symbols table information management
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _VERSION_NEEDED_H
#define _VERSION_NEEDED_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */


namespace ac_dynlink {

 /* Manages the array of needed versions required by this object
   when requesting symbol definitions from other shared object. */
  class version_needed {
  private:
    const bool match_endian;
    unsigned char *strtab;
    unsigned char *ver_needed;
    Elf32_Verneed *current_entry;
    Elf32_Vernaux *current_aux_entry;
  public:
    version_needed(unsigned char *strtab, unsigned char *ver_needed, bool match_endian);
    ~version_needed();

    bool set_entry (char * filename);
    char * get_cur_filename();
    Elf32_Half get_cur_cnt();
    Elf32_Word get_cur_hash();
    Elf32_Half get_cur_flags();
    Elf32_Half get_cur_other();
    char * get_cur_vername();
    bool go_next_aux_entry();
    char * lookup_version(Elf32_Half ver);    
  };

}

#endif
