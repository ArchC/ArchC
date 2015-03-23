/* ex: set tabstop=2 expandtab: 
   -*- Mode: C; tab-width: 2; indent-tabs-mode nil -*-
*/
/**
 * @file      opcodes.h
 * @author    Alexandro Baldassin (UNICAMP)
 *            Daniel Casarotto (UFSC)
 *            Max Schultz (UFSC)
 * 
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 *            System Design Automation Lab (LAPS)
 *            INE-UFSC
 *            http://www.laps.inf.ufsc.br/
 * 
 * @version   1.0
 * @date     Thu, 01 Jun 2006 14:28:06 -0300 
 * 
 * @brief     Opcodes library related code
 * 
 * The Opcodes module deals with the aspects involved in retargeting
 * the Opcodes library.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or 
 * (at your option) any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details. 
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/** @defgroup opcodes_group Opcodes library
 * @ingroup binutils_group
 *
 * @{
 */

#ifndef _OPCODES_H_
#define _OPCODES_H_

extern void create_operand_list();
extern void update_oper_list(int oper_id, unsigned int reloc_id);
extern int CreateOpcodeTable(const char *table_filename);
extern int CreateAsmSymbolTable(const char *symtab_filename);
extern int CreatePseudoOpsTable(const char *optable_filename);
extern int CreateOperandTable(const char *optable_filename);
extern int CreateModifierEnum(const char *filename);
extern int CreateModifierProt(const char *filename);
extern int CreateFormatStruct(const char *filename);
extern int CreateModifierPtr(const char *filename, int which);

#endif /* _OPCODES_H_ */

/* @} */

