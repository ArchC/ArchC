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

#include "memmap.H"

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
	if (prior == list)
	  list = aux;
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
    }

  /* 
     Default destructor
   */
  memmap::~memmap() {
    free_memmap();
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
  
  Elf32_Addr memmap::suggest_free_region (Elf32_Word size) {
    memmap_node *aux = list;

    while (aux->get_next() != NULL)
      aux = aux->get_next();

    return aux->get_addr();
  }
  
}
