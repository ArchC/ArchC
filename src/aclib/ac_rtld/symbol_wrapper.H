/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      symbol_wrapper.H
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
 * @brief     Provides methods to access symbol information.
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _SYMBOL_WRAPPER_H
#define _SYMBOL_WRAPPER_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include <ac_utils.H>

namespace ac_dynlink {

  /* Provides methods to access a symbol without worrying about
   how the symbol is actually written in memory (little or big endian) */
  class symbol_wrapper {
  private:
    Elf32_Sym *symbol;
    bool match_endian;
  public:
    symbol_wrapper(Elf32_Sym *sym, bool match_endian) :
      symbol(sym),
      match_endian(match_endian) {
    }

    Elf32_Word read_name_ndx() {return convert_endian(4, symbol->st_name, match_endian); }
    Elf32_Addr read_value() {return convert_endian(4, symbol->st_value, match_endian); }
    Elf32_Word read_size() {return convert_endian(4, symbol->st_size, match_endian); }
    unsigned char read_info() {return symbol->st_info; }
    unsigned char read_other() {return symbol->st_other; }
    Elf32_Section read_section_ndx() {return convert_endian(2, symbol->st_shndx, match_endian); }

    void write_name_ndx(Elf32_Word ndx) {symbol->st_name = convert_endian(4, ndx, match_endian); }
    void write_value(Elf32_Addr addr) { symbol->st_value = convert_endian(4, addr, match_endian); }
    void write_size(Elf32_Word size) {symbol->st_size = convert_endian(4, size, match_endian); }
    void write_info(unsigned char info) { symbol->st_info = info; }
    void write_other(unsigned char other) { symbol->st_other = other; }
    void write_section_ndx(Elf32_Section sec) {symbol->st_shndx = convert_endian(2, sec, match_endian); }
    
  };

}

#endif
