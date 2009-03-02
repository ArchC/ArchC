/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      version_definitions.cpp
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
 * @brief     Defined symbol versions table information management. (implementation file)
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <ac_utils.H>
#include "version_definitions.H"

namespace ac_dynlink  {
   
  /* version_definitions methods */


  version_definitions::version_definitions(unsigned char *strtab, unsigned char *ver_def, bool match_endian):
    match_endian(match_endian),
    ver_def(ver_def),
    strtab(strtab) {
    current_entry = NULL;
  }
  version_definitions::~version_definitions() {}

  bool version_definitions::set_entry(char *vername, Elf32_Word hash)
  {
    current_entry = reinterpret_cast<Elf32_Verdef *>(ver_def);
    
    while (current_entry != NULL)
      {
	if (get_cur_hash() == hash)
	  {
	    if (!strcmp(get_cur_name(), vername))
	      return true;
	  }
	if (current_entry->vd_next == 0)
	  current_entry = NULL;
	else
	  current_entry = reinterpret_cast<Elf32_Verdef *>(reinterpret_cast<char *>(current_entry) + convert_endian(4, current_entry->vd_next, match_endian));
      }
    return false;
  }

  bool version_definitions::set_entry(Elf32_Half index)
  {
    current_entry = reinterpret_cast<Elf32_Verdef *>(ver_def);
    
    while (current_entry != NULL)
      {
	if (convert_endian(2, current_entry->vd_ndx, match_endian) == index)
	  return true;
	
	if (current_entry->vd_next == 0)
	  current_entry = NULL;
	else
	  current_entry = reinterpret_cast<Elf32_Verdef *>(reinterpret_cast<char *>(current_entry) + convert_endian(4, current_entry->vd_next, match_endian));
      }
    return false;
  }

  Elf32_Word version_definitions::get_cur_hash() {
    if (current_entry == NULL) return 0;
    return convert_endian(4, current_entry->vd_hash, match_endian);
  }
  
  Elf32_Half version_definitions::get_cur_flags() {
    if (current_entry == NULL) return 0;
    return convert_endian(4, current_entry->vd_flags, match_endian);
  }
  
  char * version_definitions::get_cur_name(){
    Elf32_Verdaux *aux;
    if (current_entry == NULL) return 0;
    if (current_entry->vd_aux == 0) return 0;
    aux = reinterpret_cast<Elf32_Verdaux *>(reinterpret_cast<char *>(current_entry) + current_entry->vd_aux);
    return (char *) strtab + convert_endian(4, aux->vda_name, match_endian);
  }

}
