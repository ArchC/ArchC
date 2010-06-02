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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "ac_utils.H"


#include "ac_rtld.H"
#include "link_node.H"


namespace ac_dynlink {

  ac_rtld::ac_rtld() {
    root = NULL;
    initiated = false;
    glibc = false;
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

    /* Iterates through all loaded objects and
       copies any symbol marked by a copy relocation. */
    p = root;
    while (p != NULL) {
      p->apply_pending_copies();
      p = p->get_next();
    }
  }

  unsigned *ac_rtld::get_init_array() {
    if (root != NULL)
      return root->get_start_vector();
    else
      return NULL;
  }

  unsigned ac_rtld::get_init_arraysz() {
    if (root != NULL)
      return root->get_start_vector_n();
    else
      return 0;
  }

  unsigned *ac_rtld::get_fini_array() {
    if (root != NULL)
      return root->get_fini_vector();
    else
      return NULL;
  }

  unsigned ac_rtld::get_fini_arraysz() {
    if (root != NULL)
      return root->get_fini_vector_n();
    else
      return 0;
  }

  void  ac_rtld::set_init_arraysz(unsigned value) {
    if (root != NULL)
      root->set_start_vector_n(value);
  }
  void  ac_rtld::set_fini_arraysz(unsigned value) {
    if (root != NULL)
      root->set_fini_vector_n(value);
  }

  /* Initiate the run time dynamic linker */
  void ac_rtld::initiate(Elf32_Addr start_addr, Elf32_Word size, Elf32_Word memsize, Elf32_Word brkaddr, int fd, bool match_endian) {
    if (!initiated) {
      initiated = true;
      mem_map.set_memsize(memsize);
      mem_map.add_region(start_addr, size);
      mem_map.set_brk_addr(brkaddr);
      detect_static_glibc(fd, match_endian);
    }
  }
    
  /* Load all requested dynamic modules and link them. After this,
   we're done and simulation may start. */
  void ac_rtld::loadnlink_all(Elf32_Addr dynaddr, const char *pinterp, unsigned char *mem, Elf32_Addr& start_addr, Elf32_Word size,
			 unsigned char word_size, bool match_endian, Elf32_Word mem_size,
			 unsigned int& ac_heap_ptr) {
    unsigned *initvec, initvecn;
    this->word_size = word_size;
    this->glibc = true;
    
    if (rtld_config.is_config_loaded())
      root = new link_node(NULL, &rtld_config);
    else
      root = new link_node(NULL, NULL);
    root->set_root(root);
    root->set_program_interpreter(pinterp);
    root->link_node_setup(dynaddr, mem, 0, ET_EXEC, NULL, NULL, match_endian);
    
    load_libraries(mem, mem_size);
    
    ac_heap_ptr = mem_map.suggest_free_region(0);

    mem_map.set_brk_addr(ac_heap_ptr);
    
    link(mem);

    initvec = root->get_start_vector();
    initvecn = root->get_start_vector_n();
    if (initvecn != 0) {
      int i;
      Elf32_Addr tmp = start_addr;
      start_addr = initvec[0];
      for (i =0; i < initvecn-1; i++)
        initvec[i] = initvec[i+1];
      initvec[i] = tmp;
    }
  }
  


  bool ac_rtld::is_glibc() {
    return this->glibc;
  }
  
  
  bool ac_rtld::detect_static_glibc(int fd, bool match_endian) {
    Elf32_Ehdr    ehdr;
    Elf32_Shdr    shdr;
    unsigned i;

    lseek(fd, 0, SEEK_SET);
    
    //Test if it's an ELF file
    if ((read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) ||  // read header
        (strncmp((char *)ehdr.e_ident, ELFMAG, 4) != 0) ||          // test elf magic number
        0) {
      return false;
    }
    
    // first load the section name string table
    char *string_table = NULL;
    int   shoff = convert_endian(4,ehdr.e_shoff, match_endian);
    short shndx = convert_endian(2,ehdr.e_shstrndx, match_endian);
    short shsize = convert_endian(2,ehdr.e_shentsize, match_endian);
    
    lseek(fd, shoff+(shndx*shsize), SEEK_SET);
    if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
      return false;
    }
    
    string_table = (char *) malloc(convert_endian(4,shdr.sh_size, match_endian));
    lseek(fd, convert_endian(4,shdr.sh_offset, match_endian), SEEK_SET);
    if (read(fd, string_table, convert_endian(4,shdr.sh_size, match_endian)) !=
        (signed)convert_endian(4,shdr.sh_size, match_endian)) {
      free (string_table);
      return false;
    }
    
    for (i=0; i<convert_endian(2,ehdr.e_shnum, match_endian); i++) {
      
      lseek(fd, shoff + shsize*i, SEEK_SET);
      
      if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
        free (string_table);
        return false;
      }
      
      
      if (!strncmp(string_table+convert_endian(4,shdr.sh_name, match_endian), 
                   "__libc", 6)) {
        this->glibc = true;
        free(string_table);
        return true;
      }
    }
    free(string_table);
    return false;
  }
  
}
