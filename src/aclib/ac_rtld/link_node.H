/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      link_node.H
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
 *            symbols.
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */


#ifndef _LINK_NODE_H
#define _LINK_NODE_H

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include "dynamic_info.H"
#include "dynamic_symbol_table.H"
#include "dynamic_relocations.H"


namespace ac_dynlink {

  /* Forward class declarations */
  class version_needed;
  class memmap;
  class ac_rtld_config;
  class symbol_wrapper;

  /* Linked list of scheduled copies. These are created to
     handle copy relocations, that must be copied only after
     all relocations have been applied. */
  typedef struct _scheduled_copy_node {
    void *dst;
    void *src;
    size_t n;
    struct _scheduled_copy_node *next;
  } scheduled_copy_node;

  /* Class is a node of a linked list representing the loaded libraries and
     application executable file. */
  class link_node {
  private:
    link_node *next;
    link_node *root;
    ac_rtld_config *rtld_config;
    dynamic_info dyn_info;
    dynamic_symbol_table dyn_table;
    dynamic_relocations dyn_relocs;
    Elf32_Addr load_addr;
    scheduled_copy_node *sched_copy;
    unsigned int *startvec;
    unsigned int startvecn;
    unsigned int *finivec;
    unsigned int finivecn;
    unsigned int type;
    unsigned int needed_is_loaded;
    unsigned int has_relocations;
    unsigned char *soname;
    bool _rtld_global_patched;
    unsigned char *mem;
    const char *pinterp;
    bool match_endian;
  public:
    link_node(link_node *r, ac_rtld_config *rtld_config);
                                                
    ~link_node();

    void set_root (link_node *r);

    link_node *get_root ();

    void schedule_copy(void *dst, void *src, size_t n);

    void delete_copy_list(scheduled_copy_node *p);

    void apply_pending_copies();

    unsigned int *get_start_vector();

    unsigned int get_start_vector_n();

    unsigned int *get_fini_vector();

    unsigned int get_fini_vector_n();

    void set_start_vector_n(unsigned value);

    void set_fini_vector_n(unsigned value);

    const char *get_program_interpreter();

    void set_program_interpreter(const char*pinterp);

    void add_to_start_vector(unsigned int addr);

    void add_to_fini_vector(unsigned int addr);
    
    unsigned char * get_soname();

    Elf32_Sym *lookup_local_symbol(unsigned int hash, unsigned char *name,
                                   char *vername, Elf32_Word verhash);

    bool link_node_setup(Elf32_Addr dynaddr, unsigned char *mem,
			 Elf32_Addr l_addr, unsigned int t, unsigned char *name,
                         version_needed *verneed, bool match_endian);

    link_node * get_next();

    void set_next(link_node *n);

    link_node * new_node();

    void load_needed (memmap *mem_map, unsigned char *mem, Elf32_Word mem_size);

    void adjust_symbols(unsigned char *mem) ;

    void patch_rtld_global(symbol_wrapper *symbol);

    Elf32_Sym * find_symbol(unsigned char *name, char *vername, Elf32_Word verhash, bool exclude_root);

    void resolve_symbols() ;

    Elf32_Addr find_copy_relocation(unsigned char *symname);

    void patch_code(unsigned char *location, Elf32_Addr data, unsigned char target_size);

    void apply_relocations(unsigned char *mem, unsigned char word_size);
    
  };

}

#endif
