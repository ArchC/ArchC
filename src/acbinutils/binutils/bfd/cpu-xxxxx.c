/* ex: set tabstop=2 expandtab: */

#include "bfd.h"
#include "sysdep.h"
#include "libbfd.h"

const `bfd_arch_info_type bfd_'___arch_name___`_arch' = {
  ___word_size___,            /* bits in a word. */
  32,                         /* bits in an address. */
  8,                          /* bits in a byte. */
  `bfd_arch_'___arch_name___, /* enum bfd_architecture arch. */
  0,                          /* machine number */
  `"'___arch_name___`"',      /* arch name. */
  `"'___arch_name___`"',      /* printable name. */
  3,                          /* unsigned int section alignment power. */
  TRUE,                       /* the one and only. */
  bfd_default_compatible,
  bfd_default_scan,
  NULL
};
