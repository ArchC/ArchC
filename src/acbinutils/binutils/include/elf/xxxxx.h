#ifndef `_ELF_'___arch_name___`_H_FILE_'
#define `_ELF_'___arch_name___`_H_FILE_'

#include "elf/reloc-macros.h"

/* Generic data relocation id's */
#define BFD_GENERIC_8  10008
#define BFD_GENERIC_16 10016
#define BFD_GENERIC_32 10032

#define BFD_GENERIC_REL8  10108
#define BFD_GENERIC_REL16 10116
#define BFD_GENERIC_REL32 10132

START_RELOC_NUMBERS (`elf_'___arch_name___`_reloc_type')
___reloc_ids___
END_RELOC_NUMBERS (`R_'___arch_name___`_max')

#endif
