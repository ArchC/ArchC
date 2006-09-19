/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* ___arch_name___ ELF support for BFD.
   Copyright 2005, 2006 --- The ArchC Team.

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
