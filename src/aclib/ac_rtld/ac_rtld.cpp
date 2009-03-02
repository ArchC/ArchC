/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      ac_rtld.cpp
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
 * @brief     The ArchC ELF runtime loader. (main class implementation)
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */


#include <stdlib.h>

#include "ac_rtld.H"
#include "link_node.H"


namespace ac_dynlink {

  ac_rtld::ac_rtld() {
    root = NULL;
    word_size = 0;
  }
  
  ac_rtld::~ac_rtld() {
    delete root;
  }
  
  /* Iterates and loads all shared libraries requested by the executable program
     and further loaded shared libraries. */
  void ac_rtld::load_libraries(unsigned char *mem, Elf32_Word mem_size) {
    link_node *p = root;
    
    while (p != NULL) {
      p->load_needed(&mem_map, mem, mem_size);
      p = p->get_next();
    }
  }

  /* Resolve symbols using the loaded libraries and
     apply relocations. Code ought to be functional
     after this step.*/
  void ac_rtld::link(unsigned char *mem) {
    link_node *p = root;
    
    /* Iterates through all loaded objects in order
       to adjust their symbol tables to account for the
       load address offset. */
    while (p != NULL) {
      p->adjust_symbols(mem);
      p = p->get_next();
    }
    
    /* Iterates through all loaded objects in order
       to find any undefined symbol and, if found,
       resolve its location using the exported symbols
       from other loaded objects.*/
    p = root;
    while (p != NULL) {
      p->resolve_symbols();
      p = p->get_next();
    }
    
    /* Iterates through all loaded objects and
       patches the code applying pending dynamic
       relocations.*/
    p = root;
    while (p != NULL) {
      p->apply_relocations(mem, word_size);
      p = p->get_next();
    }
  }
    
  /* Initiate the run time dynamic linker */
  void ac_rtld::initiate(Elf32_Addr dynaddr, unsigned char *mem, Elf32_Addr start_addr, Elf32_Word size,
			 unsigned char word_size, bool match_endian, Elf32_Word mem_size,
			 unsigned int& ac_heap_ptr) {
    this->word_size = word_size;
    
    root = new link_node(NULL);
    root->set_root(root);
    root->link_node_setup(dynaddr, mem, 0, ET_EXEC, NULL, NULL, match_endian);
    
    mem_map.add_region(start_addr, size);
    
    load_libraries(mem, mem_size);
    
    ac_heap_ptr = mem_map.suggest_free_region(0);
    
    link(mem);
  }
  
}
