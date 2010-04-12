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
#include "ac_rtld_config.H"

namespace ac_dynlink {

  /* link_node class */

  /* Public methods */

  
  link_node::link_node(link_node *r, ac_rtld_config *rtld_config) {
    next = NULL;
    soname = NULL;
    needed_is_loaded = 0;
    root = r;
    startvec = NULL;
    startvecn = 0;
    finivec = NULL;
    finivecn = 0;
    mem = NULL;
    sched_copy = NULL;
    _rtld_global_patched = false;
    this->rtld_config = rtld_config;
  }

  link_node::~link_node() {
    if (finivec != NULL)
      delete [] finivec;
    if (startvec != NULL)
      delete [] startvec;
    if (next != NULL)
      delete next;
  }

  unsigned int *link_node::get_start_vector() {
    if (root == NULL || root == this) {
      return startvec;
    }
    return root->get_start_vector();
  }

  void link_node::schedule_copy(void *dst, void *src, size_t n) {
    scheduled_copy_node *p = new scheduled_copy_node;
    p->dst = dst;
    p->src = src;
    p->n = n;
    p->next = sched_copy;
    sched_copy = p;
  }

  void link_node::delete_copy_list(scheduled_copy_node *p) {
    if (p == NULL)
      return;
    delete_copy_list(p->next);
    delete p;
  }

  void link_node::apply_pending_copies() {
    scheduled_copy_node *p = sched_copy;

    while (p) {
      memcpy(p->dst, p->src, p->n);
      p = p->next;
    }

    delete_copy_list(p);
  }

  unsigned int link_node::get_start_vector_n() {
    if (root == NULL || root == this) {
      return startvecn;
    }
    return root->get_start_vector_n();
  }

  void link_node::set_start_vector_n(unsigned value) {
    if (root == NULL || root == this) {
      startvecn = value;
    } else {
      root->set_start_vector_n(value);
    }
  }

  void link_node::set_fini_vector_n(unsigned value) {
    if (root == NULL || root == this) {
      finivecn = value;
    } else {
      root->set_fini_vector_n(value);
    }
  }

  void link_node::add_to_start_vector(unsigned int addr) {
    if (root == NULL || root == this) {
      int i;
      unsigned *tmp = new unsigned[++startvecn];
      for (i = 0; i < startvecn-1; i++)
        tmp[i] = startvec[i];
      tmp[i] = addr;
      if (startvecn-1 > 0)
        delete [] startvec;
      startvec = tmp;
      return;
    }
    root->add_to_start_vector(addr);
  }

  unsigned int *link_node::get_fini_vector() {
    if (root == NULL || root == this) {
      return finivec;
    }
    return root->get_fini_vector();
  }

  unsigned int link_node::get_fini_vector_n() {
    if (root == NULL || root == this) {
      return finivecn;
    }
    return root->get_fini_vector_n();
  }

  void link_node::add_to_fini_vector(unsigned int addr) {
    if (root == NULL || root == this) {
      int i;
      unsigned *tmp = new unsigned[++finivecn];
      for (i = 0; i < finivecn-1; i++)
        tmp[i] = finivec[i];
      tmp[i] = addr;
      if (finivecn-1 > 0)
        delete [] finivec;
      finivec = tmp;
      return;
    }
    root->add_to_fini_vector(addr);
  }
  
  const char *link_node::get_program_interpreter(){
    if (root == NULL || root == this) {
      return pinterp;
    }
    return root->get_program_interpreter();
  }

  void link_node::set_program_interpreter(const char*pinterp){
    if (root == NULL || root == this) {
      this->pinterp = pinterp;
      return;
    }
    root->set_program_interpreter(pinterp);
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
      verneed_addr = 0, verdef_addr = 0, versym_addr = 0, init_addr = 0,
      init_addr_array = 0, init_addr_arraysz = 0, fini_addr = 0,
      fini_addr_array = 0, fini_addr_arraysz = 0;
    Elf32_Word pltrel = 0, relsize = 0;
    unsigned int reltype = 0;
    bool is_linux_runtime_linker = false;
    
    this->match_endian = match_endian;
    
    load_addr = l_addr;
    type = t;
    soname = name;
    this->mem = mem;
    
    dyn_info.load_dynamic_info(dynaddr, mem, match_endian);
    
    hashaddr = dyn_info.get_value(DT_HASH);
    symaddr = dyn_info.get_value(DT_SYMTAB);
    straddr = dyn_info.get_value(DT_STRTAB);
    verneed_addr = dyn_info.get_value(DT_VERNEED);
    verdef_addr = dyn_info.get_value(DT_VERDEF);
    versym_addr = dyn_info.get_value(DT_VERSYM);
    init_addr = dyn_info.get_value(DT_INIT);
    init_addr_array = dyn_info.get_value(DT_INIT_ARRAY);
    init_addr_arraysz = dyn_info.get_value(DT_INIT_ARRAYSZ);
    fini_addr = dyn_info.get_value(DT_FINI);
    fini_addr_array = dyn_info.get_value(DT_FINI_ARRAY);
    fini_addr_arraysz = dyn_info.get_value(DT_FINI_ARRAYSZ);

    if ( dyn_info.compare_library_soname ((char *)soname, get_program_interpreter())
         == 0) {
      is_linux_runtime_linker = true;
    }
    
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

    /* Configure init/fini addresses (avoid including the program
       interpreter) */
    if (!is_linux_runtime_linker && init_addr) {
      init_addr += l_addr;
      add_to_start_vector(init_addr);
    }
    if (!is_linux_runtime_linker && init_addr_arraysz && init_addr_array) {
      init_addr_array += l_addr;
      for (unsigned i = 0; i < init_addr_arraysz; i+= sizeof(Elf32_Addr)) {
        Elf32_Addr tmp = *(reinterpret_cast<Elf32_Addr *>(mem + 
                                                        init_addr_array +
                                                          i));
        tmp = convert_endian(sizeof(Elf32_Addr), tmp, match_endian);
        add_to_start_vector((unsigned) (tmp + l_addr));
      }
    }
    if (!is_linux_runtime_linker && fini_addr) {
      fini_addr += l_addr;
      add_to_fini_vector(fini_addr);
    }
    if (!is_linux_runtime_linker && fini_addr_arraysz && fini_addr_array) {
      fini_addr_array += l_addr;
      for (unsigned i = 0; i < fini_addr_arraysz; i+= sizeof(Elf32_Addr)) {
        Elf32_Addr tmp = *(reinterpret_cast<Elf32_Addr *>(mem + 
                                                        fini_addr_array +
                                                          i));
        tmp = convert_endian(sizeof(Elf32_Addr), tmp, match_endian);
        add_to_fini_vector((unsigned) (tmp + l_addr));
      }
    }

    /* Set back the adjusted value because dyn_info uses it when
       extracting needed libraries names. */
    dyn_info.set_value(DT_STRTAB, straddr);
    
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
    link_node *new_node = new link_node(root, rtld_config), *p;
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
                /* We need to change this symbol location. Later,
                 after all relocations have been applied, we copy. */
                sourceaddr = symbol->read_value();
                sourceaddr += load_addr;
                symsize = symbol->read_size();
                schedule_copy (mem+targetaddr, mem+sourceaddr, symsize);
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

  void link_node::patch_rtld_global(symbol_wrapper *symbol) {
    if (!_rtld_global_patched) {
      _rtld_global_patched = true;

      Elf32_Addr saddr = symbol->read_value();
      saddr += 984;
      (*(unsigned*)(mem + saddr)) = 0x88;
      saddr = symbol->read_value();
      saddr += 988;
      (*(unsigned*)(mem + saddr)) = 0x88;
    }
  }
  
  /* Finds a defined version of the symbol looking through 
     all loaded libraries. If exclude_root is true, skips
     root file symbols when looking for the symbol.
   */
  Elf32_Sym * link_node::find_symbol(unsigned char *name, char *vername, Elf32_Word verhash, 
                                     bool exclude_root)
  {
    link_node *p = root;
    unsigned int symhash = dyn_table.elf_hash(name);
    Elf32_Sym *the_symbol = NULL, *weak_sym = NULL;
    symbol_wrapper *symbol;

    if (exclude_root == true)
      p = p->get_next();
    
    while (p != NULL)
      {
	the_symbol = p->lookup_local_symbol(symhash, name, vername, verhash);
        if (the_symbol != NULL) {
          symbol = new symbol_wrapper(the_symbol, match_endian);
          if (ELF32_ST_BIND(symbol->read_info()) == STB_WEAK) {
            if (weak_sym == NULL)
              weak_sym = the_symbol;
            the_symbol = NULL;
          }
          delete symbol;
        }
	if (the_symbol != NULL)
	  break;
	p = p->get_next();
      }

    /* Hook for special symbols */
    if (the_symbol != NULL) {
      symbol = new symbol_wrapper(the_symbol, match_endian);

      if(strcmp("_rtld_global", (char*)name)==0) {
        patch_rtld_global(symbol);
      }

      delete symbol;
    }

    
    if (the_symbol == NULL && weak_sym != NULL)
      return weak_sym;
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

#define FETCH_RELOC_TYPE(a,b)                                              \
        if (rtld_config != NULL) {                                         \
          if (rtld_config->translate(ELF32_R_TYPE(a), &b)==-1)             \
            {                                                              \
              b = ELF32_R_TYPE(a);                                         \
            }                                                              \
        } else {                                                           \
          b = ELF32_R_TYPE(a);                                             \
        }

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
        unsigned reloc_type;
	info = dyn_relocs.read_info(i);
        FETCH_RELOC_TYPE(info, reloc_type);
	if (reloc_type != 2) /* R_xxxxx_COPY */
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
        unsigned reloc_type;
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

        FETCH_RELOC_TYPE(info, reloc_type);
	
	switch(reloc_type)
	  {
	  case 0: /* NULL */
	    break; 
	  case 1: /* RELATIVE */
            if (match_endian) {
              for (j=0; j<target_size; j++)
                *(((unsigned char*)(&aux))+j) = *(unsigned char *)(mem + location + j);
            } else {
              for (j=0; j<target_size; j++)
                *(((unsigned char*)(&aux))+j) = *(unsigned char *)(mem + location+target_size-1-j) ;
            }
            aux += load_addr;
            patch_code(mem+location, aux, target_size);
	    break;
	  case 2: /* COPY */
	    /* We copy this symbol at adjust_symbols() function. */
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
		     reloc_type << "\"." << std::endl <<"\
 If this dynamic object was not created with an ArchC generated linker, please"
                     << std::endl << "\
 include a path to locate this model's conversion map ac_rtld.relmap. To do" <<
                     std::endl << "\
 this, set the environment variable AC_LIBRARY_PATH with the path.");
	    exit(EXIT_FAILURE);
	  }
	
      } /* for(i=0;i<dyn_relocs.get_size();i++) */
  } /* apply_relocations() */

}
