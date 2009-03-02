/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      link_node.cpp
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
 * @brief     ArchC runtime dynamic linker main structure.
 *            Stores a linking unit (object file) and
 *            provides methods to relocate and bind
 *            symbols. (implementation)
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <string.h>

#include <ac_utils.H>
#include "link_node.H"
#include "symbol_wrapper.H"
#include "version_needed.H"
#include "version_definitions.H"

namespace ac_dynlink {

  /* link_node class */

  /* Public methods */

  
  link_node::link_node(link_node *r) {
    next = NULL;
    soname = NULL;
    needed_is_loaded = 0;
    root = r;
  }
  
  void link_node::set_root (link_node *r) 
  {
    root = r; 
  }

  link_node *link_node::get_root () 
  {
    return root; 
  }

  unsigned char * link_node::get_soname() 
  {
    return soname; 
  }

  Elf32_Sym *link_node::lookup_local_symbol(unsigned int hash, unsigned char *name,
					    char *vername, Elf32_Word verhash) 
  { 
    return dyn_table.lookup_symbol(hash, name, vername, verhash);
  }

  bool link_node::link_node_setup(Elf32_Addr dynaddr, unsigned char *mem,
				  Elf32_Addr l_addr, unsigned int t, unsigned char *name,
				  version_needed *verneed, bool match_endian) {
    Elf32_Addr hashaddr = 0, symaddr= 0, straddr = 0, reladdr = 0,
      verneed_addr = 0, verdef_addr = 0, versym_addr = 0;
    Elf32_Word pltrel = 0, relsize = 0;
    unsigned int reltype = 0;
    
    this->match_endian = match_endian;
    
    load_addr = l_addr;
    type = t;
    soname = name;
    
    dyn_info.load_dynamic_info(dynaddr, mem, match_endian);
    
    hashaddr = dyn_info.get_value(DT_HASH);
    symaddr = dyn_info.get_value(DT_SYMTAB);
    straddr = dyn_info.get_value(DT_STRTAB);
    verneed_addr = dyn_info.get_value(DT_VERNEED);
    verdef_addr = dyn_info.get_value(DT_VERDEF);
    versym_addr = dyn_info.get_value(DT_VERSYM);
    
    if (hashaddr) 
      hashaddr += l_addr;
    if (symaddr) 
      symaddr += l_addr;
    if (straddr) 
      straddr += l_addr;
    if (verneed_addr) 
      verneed_addr += l_addr;
    if (verdef_addr) 
      verdef_addr += l_addr;
    if (versym_addr) 
      versym_addr += l_addr;
    
    dyn_table.setup_hash(mem, hashaddr, symaddr, straddr, verdef_addr,
			 verneed_addr, versym_addr, match_endian);
    
    pltrel = dyn_info.get_value(DT_PLTREL);
    if (pltrel == 0) /* file has no relocations */
      {
	has_relocations = 0;
      }
    else
      {
	has_relocations = 1;
	reltype = (pltrel == DT_REL)? AC_USE_REL : AC_USE_RELA;
	reladdr = l_addr + dyn_info.get_value(pltrel); /* DT_REL or DT_RELA */
	if (reladdr == 0) /* No data relocations */
	  reladdr = l_addr = dyn_info.get_value(DT_JMPREL);
	relsize = dyn_info.get_value(pltrel + 1); /* DT_RELSZ or DT_RELASZ*/
	relsize += dyn_info.get_value(DT_PLTRELSZ);
	relsize /= (pltrel == DT_REL)? 8:12;

	dyn_relocs.setup(reladdr, relsize, mem, reltype, match_endian);
      }

    if (verneed != NULL)
      {
	/* we have special needs for this library*/
	if (verneed->set_entry((char *)name) == true)
	  {
              /* does not have version information, this is not the desired library */
	    if (!verdef_addr)
	      return false;
	    
	    do  /* For each verneeded aux entry, see if this library has the
		   desired version tag */
	      {
		if (dyn_table.get_verdefs()->set_entry(verneed->get_cur_vername(),
                                                         verneed->get_cur_hash()) == false)
		  { /* We don't have this version definition, see if it is
                       really necessary (not a weak def) */
		    if (!(verneed->get_cur_flags() & VER_FLG_WEAK))
		      return false; /* Library is old, reject it */
		  }
		
	      } while (verneed->go_next_aux_entry());
	  }
	
      }
    /* Library accepted */
    return true;
  }
  
  link_node * link_node::get_next() 
  {
    return next;
  }
  
  void link_node::set_next(link_node *n) 
  {
    next = n;
  }
  
  link_node * link_node::new_node() 
  {
    link_node *new_node = new link_node(root), *p;
    p = this;
    while (p->get_next() != NULL)
      p = p->get_next();
    p->set_next(new_node);
    return new_node;
  }
  
  void link_node::load_needed (memmap *mem_map, unsigned char *mem, Elf32_Word mem_size) 
  {
    if (!needed_is_loaded) {
      dyn_info.load_needed(mem_map, mem, this, mem_size, dyn_table.get_verneed());
      needed_is_loaded = 1;
    }
  }
  
  /* Walks through object's symbol table and ajusts its position
     accordingly to the object's load address. */
  void link_node::adjust_symbols(unsigned char *mem) 
  {
    unsigned int i;
    Elf32_Sym *elf_symbol;
    symbol_wrapper *symbol;
    unsigned char symbol_info;
    
    if (load_addr == 0)
      return; /* No need to adjust */
    
    for (i = 0;
	 i < dyn_table.get_num_symbols();
	 i++)
      {
	elf_symbol = dyn_table.get_symbol(i);
	symbol = new symbol_wrapper(elf_symbol, match_endian);
	symbol_info = symbol->read_info();
	
	if (ELF32_ST_TYPE(symbol_info) > STT_FUNC &&
	    ELF32_ST_TYPE(symbol_info) != STT_COMMON) {
	  delete symbol;
	  continue; /* Not a symbol type we need to adjust */
	}
	
	if (symbol->read_section_ndx() == SHN_UNDEF) {
	  delete symbol;
	  continue;  /* Symbol is undefined, no need to adjust */
	}
	
	if (ELF32_ST_TYPE(symbol_info) == STT_OBJECT &&
	    this != root)
	  { /* Check if the root node (exec file) has a copy
	       relocation against this symbol. */
	    Elf32_Addr targetaddr =
	      root->find_copy_relocation(dyn_table.get_name(symbol->read_name_ndx()));
	    if (targetaddr != 0) /* There is a copy relocation */
	      {
                Elf32_Addr sourceaddr;
                Elf32_Word symsize;
                /* We need to copy this value */
                sourceaddr = symbol->read_value();
                sourceaddr += load_addr;
                symsize = symbol->read_size();
                memcpy (mem+targetaddr, mem+sourceaddr, symsize);
		symbol->write_value(targetaddr);
		delete symbol;
		continue;
	      }
	  }
	
	/* Adjust its value */
	symbol->write_value(symbol->read_value() + load_addr);
	
	delete symbol;
      } /* for(i=0; i<dyn_table.get_num_symbols();i++) */
  } /* adjust_symbols() */
  
  /* Finds a defined version of the symbol looking through 
     all loaded libraries. If exclude_root is true, skips
     root file symbols when looking for the symbol.
   */
  Elf32_Sym * link_node::find_symbol(unsigned char *name, char *vername, Elf32_Word verhash, 
                                     bool exclude_root)
  {
    link_node *p = root;
    unsigned int symhash = dyn_table.elf_hash(name);
    Elf32_Sym *the_symbol = NULL;

    if (exclude_root == true)
      p = p->get_next();
    
    while (p != NULL)
      {
	the_symbol = p->lookup_local_symbol(symhash, name, vername, verhash);
	if (the_symbol != NULL)
	  break;
	p = p->get_next();
      }
    return the_symbol;
  }
  
  /* Walks through object's relocations, finds out if the symbol
     is undefined. If it is, Look up for its definition in other
     nodes */
  void link_node::resolve_symbols() 
  {
    unsigned int i;
    Elf32_Word info, verhash;
    Elf_Symndx symndx;
    Elf32_Sym *elf_symbol, *def_elf_symbol;
    symbol_wrapper *symbol, *def_symbol;
    char *vername;
    unsigned char symbol_info = 0, weak = 0;
    
    if (!has_relocations)
      return;
    
    for (i = 0;
	 i < dyn_relocs.get_size();
	 i++) 
      {
	info = dyn_relocs.read_info(i);
	symndx = ELF32_R_SYM(info);
	elf_symbol = dyn_table.get_symbol(symndx);
	symbol = new symbol_wrapper(elf_symbol, match_endian);
	symbol_info = symbol->read_info();
	
	if (ELF32_ST_TYPE(symbol_info) > STT_FUNC &&
	    ELF32_ST_TYPE(symbol_info) != STT_COMMON) {
	  delete symbol;
	  continue; /* Not a symbol type we need to resolve */
	}
	
	if (ELF32_ST_BIND(symbol_info) == STB_LOCAL ||
	    ELF32_ST_BIND(symbol_info) > STB_WEAK) {
	  delete symbol;
	  continue; /* Not global or weak */
	}
	
	if (ELF32_ST_BIND(symbol_info) == STB_WEAK) {
	  weak = 1;
	}
	
	if (symbol->read_section_ndx() != SHN_UNDEF) {
	    delete symbol;
	    continue;  /* Symbol is not undefined, no need to resolve */
	}
	
	vername = NULL;
	verhash = 0;
	/* Are we requesting a special version? */
	if (dyn_table.get_verneed() != NULL)
	  {
	    Elf32_Half verndx = dyn_table.get_verndx(symndx);
	    vername = dyn_table.get_verneed()->lookup_version(verndx & 0x7fff);
	    verhash = dyn_table.get_verneed()->get_cur_hash();
	  }
	
	def_elf_symbol = NULL;
        def_elf_symbol = find_symbol(dyn_table.get_name(symbol->read_name_ndx()), vername, verhash, false);
	
	if (def_elf_symbol == NULL)  /* Symbol not found. */
	  {
	    if (weak) {
	      delete symbol;
	      continue; /* Definition for this symbol is not a problem */	      
	    }
	    AC_ERROR("Run-time dynamic linker: Symbol \"" << 
		     dyn_table.get_name(symbol->read_name_ndx()) << "\" unknown.");
	    exit(EXIT_FAILURE);
	  }
	
	/* Symbol found */
	def_symbol = new symbol_wrapper(def_elf_symbol, match_endian);
	symbol->write_value(def_symbol->read_value());
	symbol->write_size(def_symbol->read_size());
	symbol->write_section_ndx(def_symbol->read_section_ndx());
	symbol->write_info(def_symbol->read_info());
	
	/* Done */
	delete def_symbol;
	delete symbol;
      } /* for(i=0;i<dyn_relocs.get_size();i++) */
  } /* resolve_symbols() */
  
  Elf32_Addr link_node::find_copy_relocation(unsigned char *symname)
  {
    unsigned int i;
    Elf32_Word info;
    Elf_Symndx symndx;
    Elf32_Sym *elf_symbol;
    symbol_wrapper *symbol;
    unsigned char *refname;
    
    if (!has_relocations)
      return 0;
    
    for (i = 0;
	 i < dyn_relocs.get_size();
	 i++) 
      {
	info = dyn_relocs.read_info(i);
	if (ELF32_R_TYPE(info) != 2) /* R_xxxxx_COPY */
	  continue; /* Not a copy relocation */
	symndx = ELF32_R_SYM(info);
	elf_symbol = dyn_table.get_symbol(symndx);
	symbol = new symbol_wrapper(elf_symbol, match_endian);
	refname = dyn_table.get_name(symbol->read_name_ndx());
	delete symbol;
	if (!strcmp((char*)symname,(char *)refname)) /* Matches */
	  return dyn_relocs.read_offset(i) + load_addr;
      }
    return 0;
  }

  /* Writes data to an address correcting endianess */
  void link_node::patch_code(unsigned char *location, Elf32_Addr data, unsigned char target_size)
  {
    unsigned int i, mask, num_bits = target_size *8;

    mask = 0;
    for (i=0; i<num_bits; i++)
      {
        mask |= 1;
        mask = mask << 1;
      }
    data = data & mask;
    if (match_endian) {
      for (i=0; i<target_size; i++)
        *(unsigned char*)(location+i) = *(((unsigned char *)&data)+i);
    } else {
      for (i=0; i<target_size; i++)
        *(unsigned char*)(location+i) = *(((unsigned char *)&data)+(target_size-1-i));
    }
  }
  
  /* Walks through object's relocation entries and relocate. */
  void link_node::apply_relocations(unsigned char *mem, unsigned char word_size)
  {
    unsigned int i, j, aux;
    Elf32_Word info;
    Elf32_Addr target, location;
    Elf_Symndx symndx;
    Elf32_Sym *elf_symbol;
    symbol_wrapper *symbol;
    Elf32_Word symsize;
    
    if (!has_relocations)
      return;
    
    for (i = 0;
	 i < dyn_relocs.get_size();
	 i++) 
      {
        unsigned char target_size = word_size;
	info = dyn_relocs.read_info(i);
	symndx = ELF32_R_SYM(info);
	elf_symbol = dyn_table.get_symbol(symndx);
	symbol = new symbol_wrapper(elf_symbol, match_endian);
	target = symbol->read_value();
	symsize = symbol->read_size();
	delete symbol;
	target += dyn_relocs.read_addend(i);
	location = dyn_relocs.read_offset(i);
	location += load_addr;
	
	switch(ELF32_R_TYPE(info))
	  {
	  case 0: /* NULL */
	    break; 
	  case 1: /* RELATIVE */
	    aux = convert_endian(word_size, *(mem + location), match_endian);
	    aux += load_addr;
            patch_code(mem+location, aux, target_size);
	    break;
	  case 2: /* COPY */
	    /* We already copied this symbol at adjust_symbols() function. */
	    break;
          case 5: /* ABS8 */
            target_size = 8;
            patch_code(mem+location, target, target_size);
            break;
	  case 6: /* ABS16 */
            target_size = 16;
            /* Fall through */
          case 7: /* ABS32 */
	  case 3: /* JUMP_SLOT */
	  case 4: /* GLOB_DAT */
            patch_code(mem+location, target, target_size);
	    break;
          case 8:  /* REL8  */
            target -= location;
            target_size = 8;
            patch_code(mem+location, target, target_size);
            break;
          case 9:  /* REL16 */
            target -= location;
            target_size = 16;
            patch_code(mem+location, target, target_size);
            break;
          case 10: /* REL32 */
            target -= location;
            patch_code(mem+location, target, target_size);
            break;
	  default:
	    AC_ERROR("Run-time dynamic linker: Unknown relocation code \"" << 
		     ELF32_R_TYPE(info) << "\"\
 If this dynamic object was not created with an ArchC generated linker, please \
 use a tool to convert its relocation codes to ArchC\'s relocation codes.");
	    exit(EXIT_FAILURE);
	  }
	
      } /* for(i=0;i<dyn_relocs.get_size();i++) */
  } /* apply_relocations() */

}
