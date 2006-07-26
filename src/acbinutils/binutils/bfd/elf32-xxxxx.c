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
#include `"share-'___arch_name___`.h"'

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


/* ArchC generic relocation routine prototype*/
static bfd_reloc_status_type
bfd_elf_archc_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED);

static bfd_reloc_status_type
generic_data_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED);



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


static bfd_reloc_status_type
generic_data_reloc (bfd *abfd,
             arelent *reloc_entry,
             asymbol *symbol,
             void *data,
             asection *input_section,
             bfd *output_bfd,
             char **error_message ATTRIBUTE_UNUSED)
{
  unsigned int relocation;
  asection *reloc_target_output_section;
  bfd_size_type octets = reloc_entry->address * bfd_octets_per_byte (abfd); 
  reloc_howto_type *howto = reloc_entry->howto;


  reloc_target_output_section = symbol->section->output_section;

  relocation = symbol->value;
  relocation += reloc_target_output_section->vma + symbol->section->output_offset;
  relocation += reloc_entry->addend;

  if (howto->pc_relative)
    relocation -= input_section->output_section->vma + input_section->output_offset;


  putbits(howto->bitsize, (char *) data + octets, relocation, ___endian_val___); 


  return bfd_reloc_ok;
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
  unsigned int relocation;
  bfd_size_type octets = reloc_entry->address * bfd_octets_per_byte (abfd); 
  bfd_vma output_base = 0;
  reloc_howto_type *howto = reloc_entry->howto;
  asection *reloc_target_output_section;
  mod_parms modifier_parms;
      
   
  /*
   * Taken from bfd_elf_generic_reloc
   * It is basicly a short circuit if the output is relocatable
   * 
   * TODO: maybe print an error message if output is relocatable?
   */
  if (output_bfd != NULL
      && (symbol->flags & BSF_SECTION_SYM) == 0
      && (! howto->partial_inplace
      || reloc_entry->addend == 0)) {
    reloc_entry->address += input_section->output_offset;
    return bfd_reloc_ok;
  }
  
  /*
   * This part is mainly copied from bfd_perform_relocation, but
   * here we use our own specific method to apply th relocation
   * Fields of HOWTO structure and their specific mappings
   *   type          --> id
   *   rightshift    --> operand_id
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
   * relocation => input variable in our scheme 
   */
  unsigned int address = (unsigned int) (input_section->output_section->vma + input_section->output_offset + reloc_entry->address);


  if (output_bfd != NULL) {
    /* TODO: deal with relocatable output?!? */
  }
  else 
    reloc_entry->addend = 0;
     
 /*
  * Overflow checking is NOT being done!!! 
  */

  unsigned int insn_image = (unsigned int) getbits(get_insn_size(operands[howto->rightshift].format_id), (char *) data + octets, ___endian_val___);


  modifier_parms.input   = relocation;
  modifier_parms.address = address;
  modifier_parms.section = symbol->section->name;

  encode_cons_field(&insn_image, &modifier_parms, howto->rightshift);

  if (modifier_parms.error) {
    _bfd_error_handler (_("Invalid relocation operation"));
  }

  putbits(get_insn_size(operands[howto->rightshift].format_id), (char *) data + octets, insn_image, ___endian_val___); 

  
  return bfd_reloc_ok;
}



#define `TARGET_'___endian_str___`_SYM'   `bfd_elf32_'___arch_name___`_vec'
#define `TARGET_'___endian_str___`_NAME'  `"elf32-'___arch_name___`"'
#define ELF_ARCH                          `bfd_arch_'___arch_name___
#define ELF_MACHINE_CODE                  EM_NONE
#define ELF_MAXPAGESIZE                   0x1
#define bfd_elf32_bfd_reloc_type_lookup   ___arch_name___`_reloc_type_lookup'
#define bfd_elf32_bfd_is_local_label_name ___arch_name___`_elf_is_local_label_name'
#define elf_info_to_howto                 ___arch_name___`_elf_info_to_howto'
#include "elf32-target.h"
