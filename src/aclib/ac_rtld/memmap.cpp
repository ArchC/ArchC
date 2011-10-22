/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      memmap.cpp
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
 * @brief     memmap and related classes implementation
 *            Manages free memory space
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <stdio.h> 
#include <unistd.h>

#include "memmap.H"

//#define DEBUG_MEMORY

namespace ac_dynlink {

  /*
     memmap_node class methods
   */
  /* 
     Default constructor
   */
  memmap_node::memmap_node(memmap_node * _next, memmap_status _status, Elf32_Addr _addr):
    next(_next),
    status(_status),
    addr(_addr) {
  }
    
  memmap_node* memmap_node::get_next() {
    return next;
  }
    
  Elf32_Addr memmap_node::get_addr() {
    return addr;
  }
    
  memmap_status memmap_node::get_status() {
    return status;
  }

  void memmap_node::set_status(memmap_status value) {
    status = value;
  }

  void memmap_node::set_next(memmap_node *next) {
    this->next = next;
  }
   
  /*
     memmap class methods
   */
  void memmap::delete_node(memmap_node *node) {
    memmap_node *aux = list, *prior = NULL;
    
    while (aux != NULL && aux != node){
      prior = aux;
      aux = aux->get_next();
    }
    if (node == list) {
      aux = list->get_next();
      delete list;
      list = aux;
    } else {
      memmap_node *aux2 = aux->get_next();
      delete aux;
      prior->set_next(aux2);
    }
  }

  void memmap::fix_consistency() {
    memmap_node *aux = list->get_next(), *prior=list;
    
    /* fixes misplaced nodes */
    while (aux != NULL) {
      if (aux->get_addr() <= prior->get_addr() &&
	  !(aux->get_status() == MS_FREE &&
	    prior->get_status() == MS_FREE)) {
	if (prior == list) {
	  list = aux;
          delete prior;
        } else
          delete_node(prior);
      }
      prior = aux;
      aux = aux->get_next();
    }
    
    /* merges contigous free block regions */
    aux = list->get_next();
    prior = list;
    while (aux != NULL) {
      if (aux->get_status() == MS_FREE &&
	  prior->get_status() == MS_FREE) {
	aux = aux->get_next();
	delete_node(prior->get_next());
      } else {
	prior = aux;
	aux = aux->get_next();
      }
    }    
  }

  void memmap::free_memmap() {
    memmap_node *aux = list;
    if (list == NULL)
      return;
    list = list->get_next();
    delete aux;
    free_memmap();
  }

  /* 
     Default constructor
   */
  memmap::memmap() {
      list = new memmap_node(NULL, MS_FREE, 0);
      pagesize = sysconf(_SC_PAGE_SIZE);
      brkaddr = 0;
      newbrkaddr = 0;
      memsize = 0;
      warning_display = true;
    }

  /* 
     Default destructor
   */
  memmap::~memmap() {
    free_memmap();
  }

  void memmap::set_memsize(Elf32_Addr memsize) {
    this->memsize = memsize;
  }


  void memmap::set_brk_addr(Elf32_Addr addr) {
    brkaddr = addr;
    newbrkaddr = addr;
  }

  memmap_node *memmap::find_region (Elf32_Addr addr) {
    memmap_node *aux = list;
    
    while (aux != NULL) {
      if (aux->get_addr() == addr)
	break;
      aux = aux->get_next();
    }
    return aux;
  }

  memmap_node *memmap::add_region (Elf32_Addr start_addr, Elf32_Word size) {
    memmap_node *aux = list, *prior = NULL;

    if (start_addr + ((unsigned)size) > memsize) {
      fprintf(stderr, "ArchC memory manager error: not enough memory in target.\n");
      fprintf(stderr, "  add_region failed: Start address = 0x%X ; Size = 0x%X",
              start_addr, size);
      fprintf(stderr, " ; Total Mem Size = 0x%X\n", memsize);
      exit(EXIT_FAILURE);
    }
    
    while (aux != NULL && aux->get_addr() < start_addr) {
      prior = aux;
      aux = aux->get_next();
    }
    
    if (prior == NULL) {
      prior = aux;
      aux = aux->get_next();
    }
    
    prior->set_next(new memmap_node(new memmap_node(aux, MS_FREE, start_addr+size), MS_USED, start_addr));
    fix_consistency();
    
    return find_region(start_addr);
  }

#define ALIGN_ADDR(align) ((align) - ((align) % pagesize) + pagesize)

  bool memmap::verify_region_availability(Elf32_Addr addr, Elf32_Word size, Elf32_Addr *next_addr)
  {
    memmap_node *aux = list;

    if (addr <= ALIGN_ADDR(newbrkaddr)) {
      if (next_addr != NULL)
        *next_addr = ALIGN_ADDR(newbrkaddr);
      return false;
    }

    /* Finds the highest region address which is also lower or equal addr*/
    while (aux->get_next() != NULL) {
      if (aux->get_next()->get_addr() > addr)
        break;
      aux = aux->get_next();
    }
    if (next_addr != NULL)
      *next_addr = 0;
    if (aux->get_status() == MS_USED) {
      if (next_addr != NULL && aux->get_next() != NULL)
        *next_addr = aux->get_next()->get_addr();
      return false; //  this region is occupied
    } else if (aux->get_next() != NULL &&
               aux->get_next()->get_status() == MS_USED) {
      if (addr + ((unsigned)size) > aux->get_next()->get_addr()) {
        if (next_addr != NULL && aux->get_next()->get_next() != NULL)
          *next_addr = aux->get_next()->get_next()->get_addr();
        return false; // not enough space
      }
    }
    if (addr + ((unsigned)size) > memsize)
      return false; // not enough space
    return true;
  }
  
  Elf32_Addr memmap::suggest_free_region (Elf32_Word size) {
    memmap_node *aux = list;

    while (aux->get_next() != NULL)
      aux = aux->get_next();

    if ((aux->get_addr() % pagesize) == 0)
      return aux->get_addr();
    else {
      return ALIGN_ADDR(aux->get_addr());
    }
  }

  Elf32_Addr memmap::suggest_mmap_region(Elf32_Word size) {
    /* Better suggest region far from stack and far from program break */
    Elf32_Addr addr = ((memsize - newbrkaddr) >> 1) + newbrkaddr;
    Elf32_Addr nextaddr;
    bool loop = false;
    addr = ALIGN_ADDR(addr);

    /* Verify availability */
    while (!verify_region_availability(addr, size, &nextaddr)) {
      if (nextaddr != 0 && nextaddr + ((unsigned)size) < memsize)
        addr = ALIGN_ADDR(nextaddr);
      else if (!loop) {
        loop = true;
        addr = ALIGN_ADDR(brkaddr);
      } else {
        if (warning_display) {
          fprintf(stderr, "ArchC memory manager warning: target ran out of memory - mmap call failed.\n");
          warning_display = false;
        }
        return (Elf32_Addr)-1;
      }
    }
    return addr;
  }

  Elf32_Addr memmap::mmap_anon(Elf32_Addr addr, Elf32_Word size) {

    if (size == 0)
      return (Elf32_Addr) -1;
    
    /* Verify alignment */
    if (addr % pagesize != 0) {
      addr = ALIGN_ADDR(addr);
    }

    if (addr != 0 && !verify_region_availability(addr, size, NULL)) {
      addr = 0;
    }

    if (addr == 0) {
      addr = suggest_mmap_region(size);
      if (addr == (Elf32_Addr) -1) {
        return addr;
      }
    }

    add_region(addr, size);
#ifdef DEBUG_MEMORY
    fprintf(stderr, "mmap region accepted: addr: %X size: %X brk: %X memsize: %X\n", addr, size, newbrkaddr, memsize);
#endif
    return addr;
  }

  bool memmap::munmap(Elf32_Addr addr, Elf32_Word size) {
    memmap_node* aux;
    if (addr == 0)
      return false;
    if (addr % pagesize != 0)
      return false;
    aux = find_region (addr);
    if (aux == NULL)
      return false;
    if (aux->get_next() != NULL) {
      if (aux->get_next()->get_addr() - aux->get_addr() <= size)
        {
          aux->set_status(MS_FREE);
          fix_consistency();
        }
    }
#ifdef DEBUG_MEMORY
    fprintf(stderr, "munmap region accepted: addr: %X size: %X brk: %X memsize: %X\n", addr, size, newbrkaddr, memsize);
#endif
    return true;
  }

  Elf32_Addr memmap::brk(Elf32_Addr addr) {
    memmap_node *aux = list;

    if (addr <= brkaddr)
      return newbrkaddr;

    if (addr <= newbrkaddr) {
      newbrkaddr = addr;
      return newbrkaddr;
    }

    if (addr >= memsize) {
      if (warning_display) {
        fprintf(stderr, "ArchC memory manager warning: target ran out of memory - brk call failed.\n");
        warning_display = false;
      }
      return newbrkaddr;
    }

    /* Finds the lowest used region address which is also higher or equal newbrkaddr*/
    while (aux != NULL) {
      if (aux->get_addr() >= newbrkaddr && aux->get_status() == MS_USED)
        break;
      aux = aux->get_next();
    }

    if (aux != NULL) { 
      if (addr >= aux->get_addr())
        return newbrkaddr;
    }

#ifdef DEBUG_MEMORY
    fprintf(stderr, "brk increment accepted: new brk: %X old brk: %X memsize: %X\n", addr, newbrkaddr, memsize);
#endif

    newbrkaddr = addr;
    return addr;
    
  }
}
