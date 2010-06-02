/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      version_needed.cpp
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
 * @brief     Needed version symbols table information management (implementation file)
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <string.h>

#include <ac_utils.H>
#include "version_needed.H"

namespace ac_dynlink {



  /* version_needed methods */
  
  
  version_needed::version_needed(unsigned char *strtab, unsigned char *ver_needed, bool match_endian):
    match_endian(match_endian),
    ver_needed(ver_needed),
    strtab(strtab) {
    current_entry = NULL;
    current_aux_entry = NULL;
  }
  
  version_needed::~version_needed() {}

  bool version_needed::set_entry (char * filename)
  {
    current_entry = reinterpret_cast<Elf32_Verneed *>(ver_needed);
    
    while(current_entry != NULL)
      {
	if (!strcmp(filename, get_cur_filename()))
	  {
	    current_aux_entry = reinterpret_cast<Elf32_Vernaux *>(reinterpret_cast<char *>(current_entry) + 
								  convert_endian(4, current_entry->vn_aux, match_endian));
	    return true;
	  }
	if (current_entry->vn_next == 0)
	  current_entry = NULL;
	else
	  current_entry = reinterpret_cast<Elf32_Verneed *>(reinterpret_cast<char *>(current_entry) + 
							    convert_endian(4, current_entry->vn_next, match_endian));
      }
    
    return false;
  }
  
  /* Return the filename (of current entry) */
  char * version_needed::get_cur_filename()
  {
    if (current_entry == NULL) return 0;
    return (char *) strtab + convert_endian(4, current_entry->vn_file, match_endian);
  }
  
  /* Return the number of associated aux entries (of current entry) */
  Elf32_Half version_needed::get_cur_cnt()
  {
    if (current_entry == NULL) return 0;
    return convert_endian(2, current_entry->vn_cnt, match_endian);
  }

  /* Get current aux entry hash value */
  Elf32_Word version_needed::get_cur_hash()
  {
    if (current_aux_entry == NULL) return 0;
    return convert_endian(4, current_aux_entry->vna_hash, match_endian);
  }
  
  /* Get current aux entry flags value */
  Elf32_Half version_needed::get_cur_flags()
  {
    if (current_aux_entry == NULL) return 0;
    return convert_endian(2, current_aux_entry->vna_flags, match_endian);
  }

  /* Get current aux entry other field */
  Elf32_Half version_needed::get_cur_other()
  {
    if (current_aux_entry == NULL) return 0;
    return convert_endian(2, current_aux_entry->vna_other, match_endian);
  }

  char * version_needed::get_cur_vername() 
  {
    if (current_aux_entry == NULL) return 0;
    return (char *) strtab + convert_endian(4, current_aux_entry->vna_name, match_endian);
  }
  
  bool version_needed::go_next_aux_entry()
  {
    if (current_aux_entry == NULL || current_aux_entry->vna_next == 0) return false;
    current_aux_entry = reinterpret_cast<Elf32_Vernaux *> (reinterpret_cast<char *>(current_aux_entry) + convert_endian(4, current_aux_entry->vna_next, match_endian));
    return true;
  }
  
  char * version_needed::lookup_version(Elf32_Half ver)
  {
    current_entry = reinterpret_cast<Elf32_Verneed *>(ver_needed);
    
    while(current_entry != NULL)
      {
	if (current_entry->vn_aux == 0)
	  current_aux_entry = NULL;
	else
	  current_aux_entry = reinterpret_cast<Elf32_Vernaux *>(reinterpret_cast<char *>(current_entry) + 
                                                                convert_endian(4, current_entry->vn_aux, match_endian));
	while (current_aux_entry != NULL)
	  {
	    if (get_cur_other() == ver) /* found! */
	      return get_cur_vername();
	    if (current_aux_entry->vna_next == 0)
	      current_aux_entry = NULL;
	    else
	      current_aux_entry = reinterpret_cast<Elf32_Vernaux *>(reinterpret_cast<char *>(current_aux_entry) +
								    convert_endian(4, current_aux_entry->vna_next, match_endian));
	  }
	
	if (current_entry->vn_next == 0)
	  current_entry = NULL;
	else
	  current_entry = reinterpret_cast<Elf32_Verneed *>(reinterpret_cast<char *>(current_entry) + 
							    convert_endian(4, current_entry->vn_next, match_endian));
      }
    return NULL;
  }

}
