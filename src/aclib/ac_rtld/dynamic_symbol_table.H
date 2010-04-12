/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_symbol_table.H
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
 * @brief     Dynamic symbol handling classes
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#ifndef _DYNAMIC_SYMBOL_TABLE_H
#define _DYNAMIC_SYMBOL_TABLE_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */



namespace ac_dynlink {
  
  /* Forward class declarations */
  class version_needed;
  class version_definitions;

  typedef Elf32_Word Elf_Symndx;
  typedef Elf32_Half Elf_Verndx;

  /* Class stores a dynamic symbol table of a shared object.
     Its internal representation follows a hash table.*/
  class dynamic_symbol_table {
  private:
    unsigned int nbuckets;
    unsigned int nchain;
    Elf_Symndx * buckets;
    Elf_Symndx * chain;
    Elf32_Sym * symtab;
    Elf32_Sym * last_match;
    Elf32_Sym * weak_match;
    bool is_unique_match;
    Elf_Verndx * versym;
    version_needed * verneed;
    version_definitions * verdefs;
    unsigned char * strtab;
    bool match_endian;
  protected:

    Elf32_Sym *check_symbol(Elf_Symndx symndx, unsigned char *name, 
                            char *vername, Elf32_Word verhash);

  public:
    dynamic_symbol_table();
    ~dynamic_symbol_table();

    unsigned int elf_hash (const unsigned char *name);

    void setup_hash(unsigned char *mem, Elf32_Addr hash_addr,
		    Elf32_Addr symtab_addr, Elf32_Addr strtab_addr, Elf32_Addr verdef_addr,
                    Elf32_Addr verneed_addr, Elf32_Addr versym_addr, bool match_endian);

    Elf32_Sym *lookup_symbol(unsigned int hash, unsigned char *name,
                             char *vername, Elf32_Word verhash); 

    unsigned int get_num_symbols() ;

    Elf32_Sym *get_symbol(unsigned int symndx);

    Elf32_Half get_verndx(unsigned int symndx);

    unsigned char *get_name (unsigned int strndx);

    version_definitions *get_verdefs();

    version_needed *get_verneed();

  };

}

#endif
