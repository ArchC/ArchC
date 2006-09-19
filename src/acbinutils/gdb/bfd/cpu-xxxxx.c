/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/* bfd back-end for ___arch_name___ support
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
 */

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
