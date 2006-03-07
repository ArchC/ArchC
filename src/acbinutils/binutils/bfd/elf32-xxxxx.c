/* ex: set tabstop=2 expandtab: */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"
#include "elf-bfd.h"
#include `"elf/'___arch_name___`.h"'
#include "libiberty.h"

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

  return bfd_elf_generic_reloc (abfd, reloc_entry, symbol, data, input_section, output_bfd, error_message);
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
