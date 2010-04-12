/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_symbol_table.cpp
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
 * @brief     Dynamic symbol handling - implementation
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <string.h>

#include <ac_utils.H>

#include "dynamic_symbol_table.H"
#include "version_needed.H"
#include "version_definitions.H"
#include "symbol_wrapper.H"

namespace ac_dynlink {

  /* dynamic_symbol_table methods */

  /* Private methods */

  /* After a symbol is fetched from the hash table, verifies
     if it is a match. */
  Elf32_Sym *dynamic_symbol_table::check_symbol(Elf_Symndx symndx, unsigned char *name, 
						char *vername, Elf32_Word verhash) {
    symbol_wrapper sym(&symtab[symndx], match_endian);
    Elf32_Sym *symbol = &symtab[symndx];

    /* No value / Symbol undefined */
    if (sym.read_value() == 0 ||
	sym.read_section_ndx() == SHN_UNDEF) 
      return NULL;

    /* Ignore non-data/non-code symbol types */
    if (ELF32_ST_TYPE (sym.read_info()) > STT_FUNC &&
	ELF32_ST_TYPE (sym.read_info()) != STT_COMMON)
      return NULL;
    
    /* Symbol is hidden, he does not want to be found,
       so do not disturb him.*/
    if (ELF32_ST_VISIBILITY (sym.read_other()) != STV_DEFAULT)
      return NULL;

    /* Not the symbol we are looking for */
    if (strcmp((char *)(strtab+sym.read_name_ndx()), (char *)name) != 0)
      return NULL;
    
    /* The name of the symbol matches. We need to verify version. Alert if
       this is the second match (more than one version exists for this
       symbol name) 
    */
    if (last_match != NULL)
      is_unique_match = false;
    
    /* Verify version */
    if (vername != NULL) /* Requesting a specific version */
      {
	/* Object has no version info */
	if (versym == NULL || verdefs == NULL) {
	  /* No version information, accept the symbol */
	}
	else {
	  /* Version string must match to accept this symbol */
	  Elf32_Half verndx = convert_endian(2, versym[symndx], match_endian);
	  if (verndx & 0x8000) /* Symbol is tagged as hidden, reject this version */
	    return NULL;
	  if (verdefs->set_entry(verndx & 0x7fff)
	      == false)
	    {
	      AC_ERROR("Run-time dynamic linker: fatal error");
	      exit(EXIT_FAILURE);
	    }
	  if (verdefs->get_cur_hash() != verhash)
	    return NULL;
	  if (strcmp(verdefs->get_cur_name(), vername))
	    return NULL;
	  /* Version matches, accept the symbol */
	}
      }
    else /* Specific version is not requested */
      {
	if (versym == NULL || verdefs == NULL) {
	  /* Neither one has version information, so accept the symbol. */
	}
	else { /* There is def. version info */
	  /* We must match only the base definition, because no version info is supplied */
	  Elf32_Half verndx = convert_endian(2, versym[symndx], match_endian);
          
	  /* Base def. is guaranteed to be index 1 and 2 */
	  if (verndx != 1 && verndx != 2)
	    {
	      /* Not base definition, but if there is only one version of this symbol,
		 accept the match later. */
	      last_match = symbol;
	      return NULL;
	    }
	  
	}
      }
    if (ELF32_ST_BIND(sym.read_info()) == STB_WEAK) {
      /* Weak... accept only if there is not another version */
      weak_match = symbol;
      return NULL;
    }
    /* Accept the symbol */
    return symbol;
  }

  /* Public methods */

  dynamic_symbol_table::dynamic_symbol_table() {
    versym = NULL;
    verneed = NULL;
    verdefs = NULL;
  }
  
  dynamic_symbol_table::~dynamic_symbol_table() {
    if (verneed != NULL)
      delete verneed;
    if (verdefs != NULL)
      delete verdefs;
  }
  
  /* ELF hashing function as defined by the ELF ABI */
  unsigned int dynamic_symbol_table::elf_hash (const unsigned char *name) {
    unsigned long int hash = 0;
    
    while (*name != '\0') {
      unsigned long int hi;
      hash  = (hash << 4) + *name++;
      hi = hash & 0xf0000000;
      hash ^= hi;
      hash ^= hi >> 24;
    }
    return hash;
  }

  void dynamic_symbol_table::setup_hash(unsigned char *mem, Elf32_Addr hash_addr,
					Elf32_Addr symtab_addr, Elf32_Addr strtab_addr, Elf32_Addr verdef_addr,
					Elf32_Addr verneed_addr, Elf32_Addr versym_addr, bool match_endian) {
    Elf_Symndx *hash = reinterpret_cast<Elf_Symndx *> (mem + hash_addr);
    
    this->match_endian = match_endian;

    nbuckets =  *hash++;
    nchain = *hash++;
    buckets = static_cast<Elf_Symndx *>(hash);
    hash += nbuckets;
    chain = static_cast<Elf_Symndx *>(hash);
    
    symtab = reinterpret_cast<Elf32_Sym *> (mem + symtab_addr);
    strtab = static_cast<unsigned char *> (mem + strtab_addr);
    
    if (verneed_addr != 0)
      verneed = new version_needed(strtab, mem + verneed_addr, match_endian);
    if (verdef_addr != 0)
      verdefs = new version_definitions(strtab, mem + verdef_addr, match_endian);
    if (versym_addr != 0)
      versym = reinterpret_cast<Elf_Verndx *> (mem + versym_addr);
  }
  
  Elf32_Sym *dynamic_symbol_table::lookup_symbol(unsigned int hash, unsigned char *name,
						 char *vername, Elf32_Word verhash) {
    Elf_Symndx symndx;
    Elf32_Sym *symbol = NULL;
    
    weak_match = NULL;
    last_match = NULL;
    is_unique_match = true;
    
    for ( symndx = buckets[hash % nbuckets];
	  symndx != STN_UNDEF;
	  symndx = chain[symndx] ) {
      symbol = check_symbol(symndx, name, vername, verhash);
      if (symbol != NULL)
	return symbol;
    }
    
    if (last_match != NULL &&
	is_unique_match)
      return last_match;

    if (weak_match)
      return weak_match;
    
    return NULL;
  }
  
  unsigned int dynamic_symbol_table::get_num_symbols() 
  {
    return nchain;
  }
  
  Elf32_Sym *dynamic_symbol_table::get_symbol(unsigned int symndx)
  {
    if (symndx >= nchain)
      return NULL;
    return &symtab[symndx];
  }
  
  Elf32_Half dynamic_symbol_table::get_verndx(unsigned int symndx)
    {
      if (symndx >= nchain)
	return 0;
      return convert_endian(2, versym[symndx], match_endian);
    }

  unsigned char *dynamic_symbol_table::get_name (unsigned int strndx)
  {
    return strtab + strndx;
  }
  
  version_definitions *dynamic_symbol_table::get_verdefs()
  {
    return verdefs;
  }
  
  version_needed *dynamic_symbol_table::get_verneed()
  {
    return verneed;
  }

}
