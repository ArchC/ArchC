/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      acrelconvert.c
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
 * @date      Thu, 01 Jun 2006 14:28:06 -0300
 * 
 * @brief     ArchC binary utilities relocation converter. Converts
 *            relocation codes in ELF files so an incompatible object
 *            file can be converted and linked against other object files created
 *            by an ArchC generated linker, and vice-versa.
 * 
 * 
 * @attention Copyright (C) 2002-2009 --- The ArchC Team
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//Fix for Cygwin users, that do not have elf.h
#if defined(__CYGWIN__) || defined(__APPLE__)
#include "elf32-tiny.h"
#else
#include <elf.h>
#endif /* __CYGWIN__ */

#include "acrelconvert.h"

int
main (int argc, char **argv)
{
  unsigned int mapfile_fd;
  hash_node **hashtable;
  char reverse_mode;

  if (hash_initialize(&hashtable) == ACRELCONVERT_FUNC_ERROR)
    exit(EXIT_FAILURE);
    
  if (process_parameters(argc, argv, &reverse_mode) == ACRELCONVERT_FUNC_ERROR)
    exit(EXIT_FAILURE);

  if ((mapfile_fd = open(argv[2], 0)) == -1) {
    fprintf(stderr, "Fatal error while opening map file \"%s\"\n", argv[2]);
    exit(EXIT_FAILURE);
  } 

  if (process_map_file(mapfile_fd, hashtable, reverse_mode) == ACRELCONVERT_FUNC_ERROR)
    {
      close(mapfile_fd);
      exit(EXIT_FAILURE);
    }
  close(mapfile_fd);

  if (process_elf(argv[3], hashtable) == ACRELCONVERT_FUNC_ERROR)
    {
      hash_delete (&hashtable);
      exit(EXIT_FAILURE);
    }

  hash_delete (&hashtable);

  exit(EXIT_SUCCESS);

}

int hash_initialize (hash_node *** hashtable)
{
  unsigned int ndx = 0;

  *hashtable = malloc (sizeof(hash_node*) * ACRELCONVERT_HASHTABLE_SIZE);

  if (*hashtable == NULL)
    {
      fprintf(stderr, "Fatal error while allocating space for hash table\n");
      return ACRELCONVERT_FUNC_ERROR;
    }

  for (ndx = 0; ndx < ACRELCONVERT_HASHTABLE_SIZE; ndx++)
    {
      (*hashtable)[ndx] = NULL;
    }

  return ACRELCONVERT_FUNC_OK;
}

int hash_add_value (hash_node **hashtable, unsigned int index, unsigned int value)
{
  hash_node *newnode;

  newnode = malloc (sizeof(hash_node));

  if (newnode == NULL)
    {
      fprintf(stderr, "Fatal error while allocating space for new hash node\n");
      return ACRELCONVERT_FUNC_ERROR;
    } 

  newnode->index = index;
  newnode->value = value;
  newnode->next = hashtable[index % ACRELCONVERT_HASHTABLE_SIZE];
  hashtable[index % ACRELCONVERT_HASHTABLE_SIZE] = newnode;

  return ACRELCONVERT_FUNC_OK;
}

void hash_delete (hash_node ***hashtable)
{
  unsigned int ndx;
  hash_node * p, * q;
  
  for (ndx = 0; ndx < ACRELCONVERT_HASHTABLE_SIZE; ndx++)
    {
      if ((*hashtable)[ndx] != NULL)
	{
	  p = ((*hashtable)[ndx])->next;
	  while (p != NULL)
	    {
	      q = p->next;
	      free(p);
	      p = q;
	    }
	  free ((*hashtable)[ndx]);
	}
    }
  free (*hashtable);
  *hashtable = NULL;
}

int hash_get_value (hash_node **hashtable, unsigned int index, unsigned int *value)
{
  hash_node *p;

  p = hashtable[index % ACRELCONVERT_HASHTABLE_SIZE];
  if (p == NULL)
    return ACRELCONVERT_FUNC_ERROR;
  while (p->index != index)
    {
      p = p->next;
      if (p == NULL)
	return ACRELCONVERT_FUNC_ERROR;
    }
  *value = p->value;
  return ACRELCONVERT_FUNC_OK;
}

int process_parameters(int argc, char **argv, char *reverse_mode)
{
  unsigned int i = 0, j = 0, size = 0;

  *reverse_mode = FALSE;
  
  /* Is anything missing? We always need 3 parameters: map file indicator(--map-file or -m),
     map file name and file name to convert */
  if (argc != 4)
    {
      fprintf(stderr, "Wrong number of parameters.\n\n");
      print_usage(stderr, argv[0]);
      return ACRELCONVERT_FUNC_ERROR;
    }

  /* Parsing */
  if (strcmp(argv[1], "--map-file") != 0 && 
      strcmp(argv[1], "-m") != 0)
    {
      if (strcmp(argv[1], "-r") == 0)
	{
	  *reverse_mode = TRUE;
	}
      else
	{
	  fprintf(stderr, "Expecting \"--map-file\" parameter.\n\n");
	  print_usage(stderr, argv[0]);
	  return ACRELCONVERT_FUNC_ERROR;
	}
    }
  return ACRELCONVERT_FUNC_OK;
}

int request_read(unsigned int fd, char *buf, unsigned int quantity, int is_fatal)
{
  if(read(fd, buf, quantity)!= quantity)
    {
      if (is_fatal)
	fprintf(stderr, "Error while reading map file: unexpected end of file\n");
      buf[0] = '\0';
      return ACRELCONVERT_FUNC_ERROR;
    }
  return ACRELCONVERT_FUNC_OK;
}

int process_map_file(unsigned int fd, hash_node **hashtable, char reverse_mode)
{
  unsigned int i, num1, num2;
  char buffer[20];
  unsigned int line = 0;

  buffer[0] = 'a';

  while (buffer[0] != '\0')
    {
      char* endptr;
      line++;

      request_read(fd, buffer, 1, FALSE);

      /* Ignore line */
      if (buffer[0] == '#')
	{
	  while (buffer[0] != '\n' && buffer[0] != '\0')
	    request_read(fd, buffer, 1, FALSE);
	  continue;
	}

      /* Skips whitespace */
      while (buffer[0] == ' ' || buffer[0] == '\t')
	  request_read(fd, buffer, 1, FALSE);

      /* Nothing to parse here, go to next line */
      if (buffer[0] == '\n' || buffer[0] == '\0')
	continue;

      /** Parse num = num statement **/
      /* Not a number */
      if (buffer[0] < '0' || buffer[0] > '9')
	{
	  fprintf(stderr, "Error while parsing map file: expecting number = number statement at line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      i = 0;

      while (buffer[i] >= '0' && buffer[i] <= '9')
	{ 
	  request_read(fd, &buffer[++i], 1, FALSE);
	  if (i >= 20)
	    {
	      fprintf(stderr, "Error while parsing map file: number too large at line %d\n", line);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	}

      /* Unexpected end of line */
      if (buffer[i] == '\n' || buffer[i] == '\0') 
	{
	  fprintf(stderr, "Error while parsing map file: unexpected end of line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      endptr = NULL;

      num1 = strtol(buffer, &endptr, 10);
      if (endptr != &buffer[i])
	{
	  fprintf(stderr, "Error while parsing map file: first number of line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}
      buffer[0] = buffer[i];

       /* Skips whitespace */
      while (buffer[0] == ' ' || buffer[0] == '\t')
	  request_read(fd, buffer, 1, FALSE);

      /* Unexpected token */
      if (buffer[0] != '=') 
	{
	  fprintf(stderr, "Error while parsing map file: expected token \"=\" at line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      request_read(fd, buffer, 1, FALSE);

      /* Skips whitespace */
      while (buffer[0] == ' ' || buffer[0] == '\t')
	request_read(fd, buffer, 1, FALSE);

      /* Unexpected end of line */
      if (buffer[0] == '\n' || buffer[0] == '\0') 
	{
	  fprintf(stderr, "Error while parsing map file: unexpected end of line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      /* Parse the second number at num = num statement *
	 /* Not a number */
      if (buffer[0] < '0' || buffer[0] > '9')
	{
	  fprintf(stderr, "Error while parsing map file: expecting number = number statement at line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      i = 0;

      while (buffer[i] >= '0' && buffer[i] <= '9')
	{ 
	  request_read(fd, &buffer[++i], 1, FALSE);
	  if (i >= 20)
	    {
	      fprintf(stderr, "Error while parsing map file: number too large at line %d\n", line);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	}

      endptr = NULL;

      num2 = strtol(buffer, &endptr, 10);
      if (endptr != &buffer[i])
	{
	  fprintf(stderr, "Error while parsing map file: second number of line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}
      buffer[0] = buffer[i];

      if (buffer[0] != '\n' && buffer[0] != '\0')
	{
	  fprintf(stderr, "Error while parsing map file: junk at end of line %d\n", line);
	  return ACRELCONVERT_FUNC_ERROR;
	}
      if (!reverse_mode)
	{
	  if (hash_add_value(hashtable, num1, num2) == ACRELCONVERT_FUNC_ERROR)
	    {
	      fprintf(stderr, "Error while parsing map file: failed to add hash value corresponding to line %d\n", line);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	}
      else /* Reverse map */
	{
	  if (hash_add_value(hashtable, num2, num1) == ACRELCONVERT_FUNC_ERROR)
	    {
	      fprintf(stderr, "Error while parsing map file: failed to add hash value corresponding to line %d\n", line);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	}

    }

  return ACRELCONVERT_FUNC_OK;
}


/* Procedure responsible for printing correct usage information
 and current version. */
void print_usage(FILE *fp, char *appname) 
{
  fprintf(fp, "\
                ArchC Relocation Code Convertion Tool, version 1\n\
 ============================================================================== \n\
  This tool is used to convert incompatible relocation codes in ELF object\n\
files created by different linkers targeting the same architecture. This \n\
happens when an ArchC generated linker is used (which has its own particular \n\
relocation codes) and you need compatibility with a target architecture ABI \n\
compliant linker.\n\
  After a successful conversion, the objects should no longer be incompatible\n\
and can be linked together. Also, DSOs (dynamic shared objects) created by\n\
third party linkers may also be loaded with ArchC loader and vice-versa.\n\
 ============================================================================== \n\
\nUsage: %s --map-file [map file] [elf file]\n\
\n  You can also substitute \"--map-file\" for \"-m\". If you want to read the \n\
map in a reverse fashion (that is, if your map converts from target A to\n\
target B, use the map to convert from B to A), use \"-r\" instead.\n\n", appname);
}


unsigned int convert_endian(unsigned int size, unsigned int num, unsigned int match_endian)
{
  unsigned char *in = (unsigned char*) &num;
  unsigned int out = 0;

  if (! match_endian) {
    for(; size>0; size--) {
      out <<= 8;
      out |= in[0];
      in++;
    }
  }
  else {
    out = num;
  }

  return out;
}

int patch_relocation_section(unsigned int fd, unsigned int size, int rel_type, hash_node **hashtable,
			     unsigned int match_endian)
{
  unsigned int i;
  unsigned int entry_size = (rel_type == DT_REL) ? sizeof(Elf32_Rel) : sizeof(Elf32_Rela);
  unsigned int total = size / entry_size;

  if (rel_type == DT_REL) /* rel relocation type*/
    {
      Elf32_Rel rel;
      unsigned int new_code, old_code, sym;
      for (i = 0; i < total; i++)
	{
	  /* Read it. */
	  if (read(fd, &rel, sizeof(rel)) != sizeof(rel))
	    {
	      fprintf(stderr, "Couldn't read REL relocation number %d.\n", i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	  /* Patch it. */
	  old_code = ELF32_R_TYPE(convert_endian(4, rel.r_info, match_endian));
	  sym = ELF32_R_SYM(convert_endian(4, rel.r_info, match_endian));
	  if (hash_get_value(hashtable, old_code, &new_code) == ACRELCONVERT_FUNC_ERROR)
	    {
	      fprintf(stderr, "Error: unrecognized relocation code 0x%X (%d), at relocation number %d\n", 
		      old_code, old_code, i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	  rel.r_info = convert_endian(4, ELF32_R_INFO(sym, new_code), match_endian);
	  /* Write it back, patched. */
	  lseek(fd, - sizeof(rel), SEEK_CUR);
	  if (write(fd, &rel, sizeof(rel)) != sizeof(rel))
	    {
	      fprintf(stderr, "Error: couldn't write REL relocation number %d.\n", i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	}
    }
  else  /* rela relocation type */ 
    {
      Elf32_Rela rela;
      unsigned int new_code, old_code, sym;
      for (i = 0; i < total; i++)
	{
	  /* Read it. */
	  if (read(fd, &rela, sizeof(rela)) != sizeof(rela))
	    {
	      fprintf(stderr, "Couldn't read RELA relocation number %d.\n", i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	  /* Patch it. */
	  old_code = ELF32_R_TYPE(convert_endian(4, rela.r_info, match_endian));
	  sym = ELF32_R_SYM(convert_endian(4, rela.r_info, match_endian));
	  if (hash_get_value(hashtable, old_code, &new_code) == ACRELCONVERT_FUNC_ERROR)
	    {
	      fprintf(stderr, "Error: unrecognized relocation code 0x%X (%d), at relocation number %d\n", 
		      old_code, old_code, i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	  rela.r_info = convert_endian(4, ELF32_R_INFO(sym, new_code), match_endian);
	  /* Write it back, patched. */
	  lseek(fd, - sizeof(rela), SEEK_CUR);
	  if (write(fd, &rela, sizeof(rela)) != sizeof(rela))
	    {
	      fprintf(stderr, "Error: couldn't write RELA relocation number %d.\n", i);
	      return ACRELCONVERT_FUNC_ERROR;
	    }
	} 
    }
  return ACRELCONVERT_FUNC_OK;
}

int process_elf(char* filename, hash_node **hashtable)
{
  Elf32_Ehdr    ehdr;
  Elf32_Shdr    shdr;
  Elf32_Phdr    phdr;
  int           fd;
  unsigned int  i, j;
  unsigned int  match_endian;
  int shoff;         /* Section header offset */
  short shsize;      /* Section header size */
  short type;        /* ELF file type */
  char sections_patched = 0;
  char dynamic_found = FALSE;
  union _endian_test {
    char bytes[4];
    unsigned int word;
  } endian_test;

  /* Open ELF file */
  if (!filename || ((fd = open(filename, O_RDWR)) == -1)) {
    perror("Fatal error - open file");
    return -1;
  }

  /* Test if it is an ELF file */
  if ((read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) ||  // read header
      (strncmp((char *)ehdr.e_ident, ELFMAG, 4) != 0) ||          // test elf magic number
      0) {
    close(fd);
    fprintf(stderr, "Failure opening file: Not an ELF file or magic number mismatches.\n");
    exit(EXIT_FAILURE);
  }

  /* Our program is running little or big endian machine? */
  endian_test.word = 1;
  if (endian_test.bytes[0] == 1)
    match_endian = 0; /* little */
  else
    match_endian = 1; /* big */

  /* Big or little? */
  if (ehdr.e_ident[EI_DATA] == ELFDATA2MSB) /* Big endian */ 
    {
      match_endian = !(match_endian ^ 1); 
    }
  else if (ehdr.e_ident[EI_DATA] == ELFDATA2LSB) /* Little endian */
    {
      match_endian = !(match_endian ^ 0); 
    } 
  else
    {
      fprintf(stderr, "Warning! Can't determine if ELF file is little or big endian. Machine default assumed.\n");
    }

  /* Testing file type */
  type = convert_endian(2, ehdr.e_type, match_endian);

  if (type != ET_EXEC && type != ET_REL && type != ET_DYN)
    {
      close(fd);
      fprintf(stderr, "Can't patch this file: this ELF is not a relocatable file, executable or DSO.\n");
      return ACRELCONVERT_FUNC_ERROR;
    }

  /* Trying to locate section header */
  shoff = convert_endian(4,ehdr.e_shoff, match_endian);
  shsize = convert_endian(2,ehdr.e_shentsize, match_endian);

  if (shoff == 0 || shsize == 0)
    { 
      unsigned int segment_type;
      short phnum = convert_endian(2, ehdr.e_phnum, match_endian);
      short phentsize = convert_endian(2, ehdr.e_phentsize, match_endian);
      int phoff = convert_endian(4, ehdr.e_phoff, match_endian);

      fprintf(stderr, "Warning: File's section header has been striped out. Can't locate ordinary relocation sections. We will still try patching dynamic relocations...\n");
      if (type != ET_DYN)
	{
	  fprintf(stderr, "Error: File is not a dynamic shared object, can't continue without section header.\n");
	  close(fd);
	  return ACRELCONVERT_FUNC_ERROR;
	}

      if (phnum == 0 || phoff == 0)
	{
	  fprintf(stderr, "Fatal error: Program headers are missing.\n");
	  close(fd);
	  return ACRELCONVERT_FUNC_ERROR;
	}
      /* Iterates through all program headers and locate the DYNAMIC segment type */
      for (i=0; i < phnum; i++)
	{
	  /* No need to continue looping if the DYNAMIC segment was already found.*/
	  if (dynamic_found)
	    break;

	  lseek(fd, phoff + phentsize * i, SEEK_SET);
	  if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
	    close(fd);
	    fprintf(stderr, "Fatal error: Couldn't read program header.\n");
	    return ACRELCONVERT_FUNC_ERROR;
	  }
	  segment_type = convert_endian(4, phdr.p_type, match_endian);
	  /* Found! */
	  if (segment_type == PT_DYNAMIC)
	    {
	      Elf32_Word p_filesz = convert_endian(4,phdr.p_filesz, match_endian), rel_size;
	      Elf32_Off  p_offset = convert_endian(4,phdr.p_offset, match_endian);

	      unsigned int rel_offset;
	      
	      Elf32_Dyn * buffer = NULL;
	      Elf32_Addr rel_addr;
	      int rel_type;

	      buffer = (Elf32_Dyn *) malloc(sizeof(char)*p_filesz);	      
	      if (buffer == NULL)
		{
		  close(fd);
		  fprintf(stderr, "Fatal error in buffer dynamic allocation.\n");
		  return ACRELCONVERT_FUNC_ERROR;
		}
	      lseek(fd, p_offset, SEEK_SET);
	      if (read(fd, buffer, p_filesz) != p_filesz)
		{
		  close(fd);
		  free(buffer);
		  fprintf(stderr, "Fatal error: Couldn't read DYNAMIC segment.\n");
		  return ACRELCONVERT_FUNC_ERROR;
		}
	      rel_type = 0;
	      rel_addr = 0;
	      rel_size = 0;
	      /* Iterates though all dynamic entries and finds REL/RELA records */
	      for (j = 0; j < (p_filesz / sizeof(Elf32_Dyn)); j++)
		{
		  switch (convert_endian(4,buffer[j].d_tag, match_endian))
		    {
		    case DT_REL:
		    case DT_RELA:
		      rel_type = convert_endian(4, buffer[j].d_tag, match_endian);
		      rel_addr = convert_endian(4, buffer[j].d_un.d_ptr, match_endian);
		      break;
		    case DT_RELSZ:
		    case DT_RELASZ:
		      rel_size = convert_endian(4, buffer[j].d_un.d_val, match_endian);
		      break;
		    default:
		      break;
		    }
		}
	      free(buffer);

	      /* If any info is missing, terminate */
	      if (rel_type == 0 || rel_addr == 0 || rel_size == 0)
		{
		  close(fd);
		  fprintf(stderr, "Could not locate a dynamic relocation section in this DSO.\n");
		  return ACRELCONVERT_FUNC_ERROR;
		}

	      rel_offset = 0;

	      /* Identify where, in the file, these relocations are (rel_offset) */
	      for (j=0; j < phnum; j++)
		{
		  lseek(fd, phoff + phentsize * i, SEEK_SET);
		  if (read(fd, &phdr, sizeof(phdr)) != sizeof(phdr)) {
		    close(fd);
		    fprintf(stderr, "Fatal error: Couldn't read program header.\n");
		    return ACRELCONVERT_FUNC_ERROR;
		  }
		  segment_type = convert_endian(4, phdr.p_type, match_endian);      
		  if (segment_type == PT_LOAD)
		    {
		      Elf32_Addr p_vaddr = convert_endian(4,phdr.p_vaddr, match_endian);
		      Elf32_Word p_memsz = convert_endian(4,phdr.p_memsz, match_endian);
		      Elf32_Word p_filesz = convert_endian(4,phdr.p_filesz, match_endian);
		      Elf32_Off  p_offset = convert_endian(4,phdr.p_offset, match_endian);

		      /* Does this segment contains our dynrelocs section? */
		      if (p_vaddr <= rel_addr && p_vaddr + p_filesz >= rel_addr + rel_size)
			{
			  rel_offset = p_offset + (rel_addr - p_vaddr);
			}
		    }
		}
	      /* Not found */
	      if (rel_offset == 0)
		{
		  close(fd);
		  fprintf(stderr, "Could not locate the dynamic relocation section in this DSO.\n");
		  return ACRELCONVERT_FUNC_ERROR;
		}

	      /* Now we have rel_offset, rel_type and rel_size. Start patching relocations */
	      dynamic_found = TRUE;

	      lseek(fd, rel_offset, SEEK_SET);

	      if (patch_relocation_section(fd, rel_size, rel_type, hashtable, match_endian)
		  == ACRELCONVERT_FUNC_ERROR)
		{
		  close(fd);
		  fprintf(stderr, "Failed patching relocation codes at dynrelocs section.\n");
		  return ACRELCONVERT_FUNC_ERROR;
		}
	    }
	}
      if (!dynamic_found)
	{
	  close(fd);
	  fprintf(stderr, "Fatal error: Couldn't find any DYNAMIC segment in this DSO, preventing us from locating dynamic relocations section without a section headers table.\n");
	  return ACRELCONVERT_FUNC_ERROR;
	}
      
    }
  else /* We do have section headers information. */
    {
      int   shoff = convert_endian(4,ehdr.e_shoff, match_endian);
      short shsize = convert_endian(2,ehdr.e_shentsize, match_endian);
   
      
      /* Iterates though all sections. Patches all REL or RELA section types. */
      for (i=0; i<convert_endian(2,ehdr.e_shnum, match_endian); i++) {
	Elf32_Word shtype;
	int rel_type, rel_size;
	
	lseek(fd, shoff + shsize*i, SEEK_SET);
	
	if (read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)) {
	  fprintf(stderr, "Error while trying to read section header.\n");
	  close(fd);
	  return ACRELCONVERT_FUNC_ERROR;
	}

	shtype = convert_endian(4, shdr.sh_type, match_endian);
	if (shtype == SHT_REL || shtype == SHT_RELA)
	  {
	    if (shtype == SHT_REL)
	      rel_type = DT_REL;
	    else
	      rel_type = DT_RELA;
	    rel_size = convert_endian(4, shdr.sh_size, match_endian);
	    lseek(fd, convert_endian(4, shdr.sh_offset, match_endian), SEEK_SET);

	    if (patch_relocation_section(fd, rel_size, rel_type, hashtable, match_endian)
		== ACRELCONVERT_FUNC_ERROR)
	      {
		close(fd);
		fprintf(stderr, "Failed patching relocation codes at section number %d.\n", i);
		return ACRELCONVERT_FUNC_ERROR;
	      }
	    sections_patched++;
	  }
      }

    }

  if (!sections_patched)
    {
      if (!dynamic_found)
	printf("\nNo relocation sections found. File has not been changed.\n");
    }
  else
    {
      printf("\nDone patching %d sections.\n", sections_patched);
    }

  /* Close file */
  close(fd);

  return ACRELCONVERT_FUNC_OK;
}


