/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* `tc-'___arch_name___`.h' -- Header file for `tc-'___arch_name___`.c'
   Copyright 2005, 2006 --- The ArchC Team
 
   This file is automatically retargeted by ArchC binutils generation 
   tool. This file is part of GAS, the GNU Assembler.

   GAS is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GAS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GAS; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/*
 * written by:
 *   Alexandro Baldassin            
 */


#ifndef `_TC_'___arch_name___`_H_FILE_'
#define `_TC_'___arch_name___`_H_FILE_'

#define `AC_'___endian_str___`_ENDIAN'
#define AC_WORD_SIZE ___word_size___
#define TARGET_BYTES_BIG_ENDIAN ___endian_val___
#define TARGET_ARCH `bfd_arch_'___arch_name___
#define TARGET_FORMAT `"elf32-'___arch_name___`"'
#define WORKING_DOT_WORD
#define LOCAL_LABELS_FB 1
#define LOCAL_LABELS_DOLLAR 1
#define LOCAL_LABEL_PREFIX '$'

/*
typedef struct fix_addend {
  unsigned int format_id;
  unsigned int operand_id;
} archc_fix_addend;
*/

#define TC_FIX_TYPE unsigned int
#define TC_INIT_FIX_DATA(fixP)

extern char *___arch_name___`_canonicalize_symbol_name'(char *c);
#define tc_canonicalize_symbol_name(x) ___arch_name___`_canonicalize_symbol_name(x)'

extern int ___arch_name___`_parse_name'(char *name, expressionS *expP, char *c);
#define md_parse_name(x, y, z) ___arch_name___`_parse_name(x, y, z)'

extern void ___arch_name___`_frob_label'(symbolS *sym);
#define tc_frob_label(x) ___arch_name___`_frob_label'(x)

extern void ___arch_name___`_symbol_new_hook'(symbolS *sym);
#define tc_symbol_new_hook(x) ___arch_name___`_symbol_new_hook'(x)

extern void ___arch_name___`_handle_align'(struct frag *);
#define HANDLE_ALIGN(fragp) ___arch_name___`_handle_align'(fragp)

extern void ___arch_name___`_cons_fix_new'(struct frag *, int, unsigned int, struct expressionS *);
#define TC_CONS_FIX_NEW ___arch_name___`_cons_fix_new'

#define TC_VALIDATE_FIX(fixp, this_segment, skip_label) \
  do \
    if (!___arch_name___`_validate_fix'((fixp), (this_segment))) \
      goto skip_label; \
  while (0)
extern int ___arch_name___`_validate_fix'(struct fix *, asection *);

extern void ___arch_name___`_adjust_reloc_count'(struct fix *, long);
#define TC_ADJUST_RELOC_COUNT(fixP, seg_reloc_count) ___arch_name___`_adjust_reloc_count'(fixP, seg_reloc_count)


#define DIFF_EXPR_OK            /* foo-. gets turned into PC relative relocs */
#endif
