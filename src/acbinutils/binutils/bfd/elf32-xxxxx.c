/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* ___arch_name___`-specific' support for ELF
   Copyright 2005, 2006 --- The ArchC Team

This file is automatically retargeted by ArchC binutils generation tool. 
This file is part of BFD, the Binary File Descriptor library.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

/*
 * written by:
 *   Alexandro Baldassin
 *   Daniel Casarotto 
 */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include `"elf/'___arch_name___`.h"'
#include "libiberty.h"

/*
 BFD prior to version 2.16 does not include bfd_get_section_limit
*/
#ifndef bfd_get_section_limit
#define bfd_get_section_limit(bfd, sec) \
  (((sec)->_cooked_size) \
   / bfd_octets_per_byte (bfd))
#endif

static reloc_howto_type* ___arch_name___`_reloc_type_lookup' (bfd *, bfd_reloc_code_real_type);
static void ___arch_name___`_elf_info_to_howto' (bfd *, arelent *, Elf_Internal_Rela *);
static long long getbits(unsigned int bitsize, char *location, int endian); 
static void putbits(unsigned int bitsize, char *location, long long value, int endian);


/* ArchC generic relocation routine prototype*/
static bfd_reloc_status_type
bfd_elf_archc_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED);


/* carry relocation function prototype */
bfd_reloc_status_type
`_bfd_'___arch_name___`_elf_carry_reloc' (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
           void *data, asection *input_section, bfd *output_bfd, char **error_message);

/* Local label (those who starts with $ are recognized as well) */
static bfd_boolean
___arch_name___`_elf_is_local_label_name' (bfd *abfd, const char *name)
{
  if (name[0] == '$')
    return TRUE;

  return _bfd_elf_is_local_label_name(abfd, name);
}

static reloc_howto_type `elf_'___arch_name___`_howto_table'[] =
{
___reloc_howto___
};

struct ___arch_name___`_reloc_map'
{
  bfd_reloc_code_real_type bfd_reloc_val;
  unsigned int ___arch_name___`_reloc_val';
};

static const struct ___arch_name___`_reloc_map' ___arch_name___`_reloc_map'[] =
{
___reloc_map___
};

static reloc_howto_type* ___arch_name___`_reloc_type_lookup' (abfd, code)
  bfd * abfd ATTRIBUTE_UNUSED;
  bfd_reloc_code_real_type code;
{
  unsigned int i;
  for (i = ARRAY_SIZE (___arch_name___`_reloc_map'); --i;)
    if (___arch_name___`_reloc_map'[i].bfd_reloc_val == code)
      return & `elf_'___arch_name___`_howto_table'[___arch_name___`_reloc_map'[i].___arch_name___`_reloc_val'];

  return NULL;
}

static void ___arch_name___`_elf_info_to_howto' (abfd, cache_ptr, dst)
  bfd *abfd ATTRIBUTE_UNUSED;
  arelent *cache_ptr;
  Elf_Internal_Rela *dst;
{
  unsigned int r;
  r = ELF32_R_TYPE (dst->r_info);

  BFD_ASSERT (r < (unsigned int) `R_'___arch_name___`_max');

  cache_ptr->howto = &`elf_'___arch_name___`_howto_table'[r];
}

bfd_reloc_status_type
`_bfd_'___arch_name___`_elf_carry_reloc' (bfd *abfd, arelent *reloc_entry, asymbol *symbol,
           void *data, asection *input_section, bfd *output_bfd, char **error_message)
{
  if (output_bfd == NULL) { // If it's the final link, apply the carry
    bfd_vma vallo;
    bfd_byte *location = (bfd_byte *) data + reloc_entry->address;
    vallo = bfd_get_32 (abfd, location);

    unsigned long mask1 = 1 << (32 - reloc_entry->howto->bitsize - 1);
    unsigned long mask2 = 0;
    unsigned i;
    for (i = 0; i < (32 - reloc_entry->howto->bitsize); i++)
      mask2 = mask2 | 1 << i;

    reloc_entry->addend += (vallo + mask1) & mask2;
  }

  return bfd_elf_archc_reloc (abfd, reloc_entry, symbol, data, input_section, output_bfd, error_message);
}


static bfd_reloc_status_type
bfd_elf_archc_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED)
{
  bfd_vma relocation;
  bfd_size_type octets = reloc_entry->address * bfd_octets_per_byte (abfd); 
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;
//  asymbol *symbol = *(reloc_entry->sym_ptr_ptr);
        
   
  /*
   * Taken from bfd_elf_generic_reloc
   * It is basicly a short circuit if the output is relocatable
   * 
   * TODO: maybe print an error message if output is relocatable?
   */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! reloc_entry->howto->partial_inplace
      || reloc_entry->addend == 0))
     {
       reloc_entry->address += input_section->output_offset;
       return bfd_reloc_ok;
     }
  
  /*
   * This part is mainly copied from bfd_perform_relocation, but
   * here we use generic get/put bits
   * Field 'size' of HOWTO structure now indicates the -bit size-
   */

  /* Is the address of the relocation really within the section?  */
  if (reloc_entry->address > bfd_get_section_limit (abfd, input_section))
    return bfd_reloc_outofrange;

  /* Work out which section the relocation is targeted at and the
   * initial relocation command value.  */

  /* Get symbol value.  (Common symbols are special.)  */
  if (bfd_is_com_section (symbol->section))
    relocation = 0;
  else
    relocation = symbol->value;

  reloc_target_output_section = symbol->section->output_section;

  /* Convert input-section-relative symbol value to absolute.  */
  if ((output_bfd && ! howto->partial_inplace)
      || reloc_target_output_section == NULL)
    output_base = 0;
  else
    output_base = reloc_target_output_section->vma;
   
  relocation += output_base + symbol->section->output_offset;

  /* Add in supplied addend.  */
  relocation += reloc_entry->addend;

  /* Here the variable relocation holds the final address of the
   * symbol we are relocating against, plus any addend.  
   */


  if (howto->pc_relative) {
    relocation -= input_section->output_section->vma + input_section->output_offset;
     
    if (howto->pcrel_offset)
      relocation -= reloc_entry->address;
  }
     
  if (output_bfd != NULL) {
    /* TODO: deal with relocatable output?!? */
  }
  else 
    reloc_entry->addend = 0;
     
 /*
  * Overflow checking is NOT being done!!! 
  */



  relocation >>= (bfd_vma) howto->rightshift;

  /* Shift everything up to where it's going to be used.  */
  relocation <<= (bfd_vma) howto->bitpos;

  /*  
   * Now the real stuff... get, apply and put back the relocation from/into 
   * object file image
   */

  
#define DOIT(x) \
     x = ( (x & ~howto->dst_mask) | (((x & howto->src_mask) +  relocation) & howto->dst_mask))
  
  long long x;

  x = getbits(howto->size, (char *) data + octets , ___endian_val___);

  DOIT (x);

  putbits(howto->size, (char *) data + octets, x, ___endian_val___); 

  
  
  return bfd_reloc_ok;
}

/*
 * 1 = big, 0 = little
 *
 * limitations:
 * . endianess affects 8-bit bytes
 * . maximum word size: 64-bit
 */
static long long getbits(unsigned int bitsize, char *location, int endian)
{
  long long data = 0;
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
	
  if (bits_remain) number_bytes++;

  int index, inc;
  if (endian == 1) { /* big */
    index = 0;
    inc = 1;		
  }
  else { /* little */
    index = number_bytes - 1;
    inc = -1;
  }
	
  while (number_bytes) {
    data = (data << 8) | (unsigned char)location[index];
		
    index += inc;
    number_bytes--;
  }	

/*
 * if bitsize is not multiple of 8 then clear/pack the remaining bits
 */
  long long mask = 0;
  for (; bitsize; bitsize--) 
    mask = (mask << 1) | 1;
	
  if (bits_remain) {
    if (endian == 1) {
      long long temp = data >> (8 - bits_remain);
      temp &= (mask << bits_remain);
			
      data = temp | (data & ~(mask << bits_remain));
    }
  }
	
  return data & mask;
}


static void putbits(unsigned int bitsize, char *location, long long value, int endian)
{
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
  unsigned char bytes[8];
  unsigned int i;
  int index;
  unsigned char mask = 0;

  /*
	* Fill the bytes array so as not to depend on the host endian
	*
	* bytes[0] -> least significant byte
	*/
  for (i=0; i<8; i++) 
    bytes[i] = (value & (0xFF << (i*8))) >> 8*i;

  if (bits_remain) {
    for (i=0; i<bits_remain; i++)
	   mask = (mask << 1) | 1;	
  }
	 
  if (endian == 0) { /* little */
    index = 0;

    while ((unsigned int)index != number_bytes) {    
      location[index] = bytes[index];
	  
      index++;
    }

	 if (bits_remain) 
     location[number_bytes] = (location[number_bytes] & ~mask) | (bytes[number_bytes] & mask);
	 	 
  }
  else { /* big */
    index = number_bytes - 1;
	
    i = 0;	 
    while (index >= 0) {    
      if (bits_remain) 
        location[i] = ((bytes[index+1] & mask) << (8-bits_remain)) | (bytes[index] >> bits_remain);
      else		 
        location[i] = bytes[index];

      i++;
      index--;
    }

    if (bits_remain) 
      location[number_bytes] = (bytes[0] & mask) | (location[number_bytes] & ~mask);
  }	
}


#define TARGET_BIG_SYM                    `bfd_elf32_'___arch_name___`_vec'
#define TARGET_BIG_NAME                   `"elf32-'___arch_name___`"'
#define ELF_ARCH                          `bfd_arch_'___arch_name___
#define ELF_MACHINE_CODE                  EM_NONE
#define ELF_MAXPAGESIZE                   0x1
#define bfd_elf32_bfd_reloc_type_lookup   ___arch_name___`_reloc_type_lookup'
#define bfd_elf32_bfd_is_local_label_name ___arch_name___`_elf_is_local_label_name'
#define elf_info_to_howto                 ___arch_name___`_elf_info_to_howto'
#include "elf32-target.h"
