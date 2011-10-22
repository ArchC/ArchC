/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*- */

/**
 * @file      dynamic_info.cpp
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
 * @brief     dynamic_info class implementation
 *
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <ac_utils.H>
#include "dynamic_info.H"
#include "memmap.H"
#include "link_node.H"


namespace ac_dynlink {
  /* Private methods */
  
  /* Given a library name, find its location and open it. Return a descriptor 
     to the open file.*/
  int dynamic_info::find_library (const char *soname) 
  {
    int fd;
    unsigned int i, j, k;
    char *envpath, *apath;
    fd = open(soname,0);
    if (fd > 0)
      return fd;
    envpath = getenv(ENV_AC_LIBRARY_PATH);
    if (envpath == NULL)
      return -1;
    /* Process environment variable containing libraries search path */
    apath = new char[strlen(envpath) + strlen(soname) + 2];
    for (i = 0, j = 0; i < strlen(envpath); i++)
      {
	apath[j++] = envpath[i]; 
	if (envpath[i+1] == ':' || envpath[i+1] == '\0')
	  {
	    i++;
	    apath[j++] = '/';
	    for (k=0; k < strlen(soname); k++)
	      {
		apath[j++] = soname[k];
	      }
	    apath[j] = '\0';
	    fd = open(apath, 0);
	    if (fd > 0)
	      {
		delete [] apath;
		return fd;
	      }
	    j = 0;
	  }           
      }
    delete [] apath;
    return -1;
  }

  /* Verifies if two library names references the same so */
  int dynamic_info::compare_library_soname (const char *soname1, const char *soname2)
  {
    int ndx1, ndx2;
    if (soname1 == NULL && soname2 == NULL)
      return 0;
    else if (soname1 == NULL || soname2 == NULL)
      return 1;

    /* Remove path */
    for (ndx1 = strlen(soname1)-1; ndx1 >=0; ndx1--) {
      if (soname1[ndx1] == '/')
        {
          break;
        }
    }
    ndx1++;
    for (ndx2 = strlen(soname2)-1; ndx2 >=0; ndx2--) {
      if (soname2[ndx2] == '/')
        {
          break;
        }
    }
    ndx2++;

    /* Compare "filenames" (without path) */
    return strcmp(&soname1[ndx1], &soname2[ndx2]);
  }
  
  /* Public methods */

  dynamic_info::dynamic_info() {
    dynamic_segment = NULL;
  }
  dynamic_info::~dynamic_info() {
    if (dynamic_segment != NULL)
      delete dynamic_segment;
  }

  /* Given a tag in the dynamic table (DT_NEEDED, DT_HASH, etc),
     retrives its value*/
  Elf32_Word dynamic_info::get_value(Elf32_Sword tag) {
    unsigned int i;
    Elf32_Dyn *entry;

    for (i = 0, entry = dynamic_segment;
	 i < dynamic_size;
	 i++, entry++) {
      if (entry->d_tag == tag)
	break;
    }
    if (i >= dynamic_size)
      return 0;
    else
      return entry->d_un.d_val;
  }

  /* Given a tag in the dynamic table, the first appearance of it
     will receive a new value. If no tags are found, return false. */
  bool dynamic_info::set_value(Elf32_Sword tag, Elf32_Word value) {
    unsigned int i;
    Elf32_Dyn *entry;
    
    for (i = 0, entry = dynamic_segment;
	 i < dynamic_size;
	 i++, entry++) {
      if (entry->d_tag == tag)
	break;
    }
    if (i >= dynamic_size)
      return false;
    else {
      entry->d_un.d_val = value;
      return true;
    }
  }

  /* Loads the dynamic table. Needs the DYNAMIC segment address */
  void dynamic_info::load_dynamic_info (Elf32_Addr addr, unsigned char *mem, bool match_endian) {
    Elf32_Dyn *buffer;
    Elf32_Dyn *entry;
    unsigned int i;
    
    this->match_endian = match_endian;
    
    buffer = reinterpret_cast<Elf32_Dyn *>(mem + addr);
    
    dynamic_size = 0;
    while (buffer[dynamic_size++].d_tag != DT_NULL)
      ;
    
    dynamic_segment = new Elf32_Dyn[dynamic_size];
    
    for (i = 0, entry = dynamic_segment;
	 i < dynamic_size;
	 i++, entry++) {
      entry->d_tag = convert_endian(4, buffer[i].d_tag, match_endian);
      entry->d_un.d_val = convert_endian(4, buffer[i].d_un.d_val, match_endian);
    }
  }

  /* Load needed shared libraries, as indicated in DT_NEEDED tags */
  void dynamic_info::load_needed (memmap *mem_map, unsigned char *mem, link_node * l_node, Elf32_Word mem_size,
				  version_needed * verneed)
  {
    unsigned int i;
    unsigned char *soname;
    Elf32_Dyn *entry;
    
    for (i = 0, entry = dynamic_segment;
         i < dynamic_size;
         i++, entry++) {
      if (entry->d_tag == DT_NEEDED)
        {
          Elf32_Addr load_addr = mem_map->suggest_free_region(0), dyn_addr = 0;
          Elf32_Word dyn_size = 0;
          link_node *p;
          soname = static_cast<unsigned char*>(mem + get_value(DT_STRTAB) + entry->d_un.d_val);
          
          /* Verifies if library is already loaded */
          p = l_node->get_root();
          while (p != NULL) { 
            if (!compare_library_soname((char *)soname, (char *)p->get_soname()))
              break;
            p = p->get_next();
          }
          if (p)
            continue; /* library is already loaded */
          
          mem_map->add_region(load_addr, load_library(load_addr, mem, soname, dyn_addr, dyn_size, mem_size));
          
          if (dyn_addr == 0 || dyn_size == 0) {
            AC_ERROR("Run-time dynamic linker: Could not find DYNAMIC segment of library \"" << soname << "\".\n");
            exit(EXIT_FAILURE);
          }
          
          if (l_node->new_node()->link_node_setup(dyn_addr, mem, load_addr, ET_DYN,
						  soname, verneed, match_endian) == false)
	    {
	      /* Library was rejected because it is old */
	      AC_ERROR("run-time dynamic linker: Loaded library \"" << soname << "\"is \
old and can't be used (version mismatch).\n");
	      exit(EXIT_FAILURE);
	    }
        } /* if DT_NEEDED */
    } /* for (walks through all dynamic entries) */
  } /* dynamic_info::load_needed */

    /* Loads the library "soname" into app.memory "mem". Return the size occupied by the library,
       loaded at address "load_addr". If a DYNAMIC segment is present, "dyn_addr" and "dyn_size"
       by reference parameters are filled with its address and size.*/
  Elf32_Word dynamic_info::load_library (Elf32_Addr load_addr, unsigned char *mem, unsigned char *soname,
					 Elf32_Addr& dyn_addr, Elf32_Word& dyn_size, Elf32_Word mem_size) {
    Elf32_Ehdr    ehdr;
    Elf32_Shdr    shdr;
    Elf32_Phdr    phdr;
    int           fd;
    unsigned int  i;
    Elf32_Word    total_size = 0;
    
    //Open application
    if (!soname || ((fd = find_library((char *)soname)) == -1)) {
      AC_ERROR("Run-time dynamic linker: Could not find shared library \"" << soname << "\"." << std::endl << "Please properly configure the environment variable AC_LIBRARY_PATH.");
      exit(EXIT_FAILURE);
    }
    
    //Test if it's an ELF file
    if ((read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) ||  // read header
	(strncmp((char *)ehdr.e_ident, ELFMAG, 4) != 0) ||          // test elf magic number
	0) {
      close(fd);
      return EXIT_FAILURE;
    }
    
    if (convert_endian(2,ehdr.e_type, match_endian) != ET_DYN) {
      AC_ERROR("Run-time dynamic linker: File \"" << soname << "\" is not an ELF dynamic library.");
      exit(EXIT_FAILURE);
    }
    
    //It is an ELF file
#ifdef DEBUG
    printf("ArchC: Reading ELF requested dynamic library: %s@0x%X\n", soname, load_addr);
#else
    AC_SAY("Reading ELF requested dynamic library: " << soname);
#endif
    
    //Get program headers and load segments
    for (i=0; i<convert_endian(2,ehdr.e_phnum, match_endian); i++) {
      unsigned int segment_type;
      
      //Get program headers and load segments
      lseek(fd, convert_endian(4,ehdr.e_phoff, match_endian) + convert_endian(2,ehdr.e_phentsize, match_endian) * i, SEEK_SET);
      if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
	AC_ERROR("reading ELF program header.");
	close(fd);
	exit(EXIT_FAILURE);
      }
      
      segment_type = convert_endian(4, phdr.p_type, match_endian);
      
      switch(segment_type) {
      case PT_INTERP:
	break;
      case PT_DYNAMIC:  // Dynamic information
	dyn_addr = load_addr + convert_endian(4, phdr.p_vaddr, match_endian);
	dyn_size = convert_endian(4, phdr.p_memsz, match_endian);
	/* Fall through. */
      case PT_LOAD: { // Loadable segment type - load dynamic segments as well
	Elf32_Word j;
	Elf32_Addr p_vaddr = convert_endian(4,phdr.p_vaddr, match_endian);
	Elf32_Word p_memsz = convert_endian(4,phdr.p_memsz, match_endian);
	Elf32_Word p_filesz = convert_endian(4,phdr.p_filesz, match_endian);
	Elf32_Off  p_offset = convert_endian(4,phdr.p_offset, match_endian);
        ssize_t rbytes;
	
	//Error if segment greater then memory
	if (mem_size < p_vaddr + p_memsz + load_addr) {
	  AC_ERROR("not enough memory in ArchC model to load requested shared library.");
	  close(fd);
	  exit(EXIT_FAILURE);
	}
	
	if (p_vaddr + p_memsz > total_size)
	  total_size = p_vaddr + p_memsz;
	
	//Load
	lseek(fd, p_offset, SEEK_SET);
	if (read(fd, mem + p_vaddr + load_addr, p_filesz) != p_filesz)
          {
            AC_ERROR("read file operation failed.");
            close(fd);
            exit(EXIT_FAILURE);
          }
        memset(mem + p_vaddr + load_addr + p_filesz, 0, p_memsz - p_filesz);
	
	break;
      }
      default:
	break;
      }
    }
    close(fd);
    
    return total_size;
  }
  
}
