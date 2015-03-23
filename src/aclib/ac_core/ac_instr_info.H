/**
 * @file      ac_instr_info.H
 * @author    Thiago Massariolli Sigrist
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Thu May 12 13:54:55 2005
 * 
 * @brief     Defines a struct containing information about an instruction.
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

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_INSTR_INFO_H_
#define _AC_INSTR_INFO_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

/// Struct containing information about a generic ArchC instruction.
/// Each entry of the processor ISA instruction table has type
/// ac_instr_info.
struct ac_instr_info {

  /// Instruction number.
  int ac_instr_number;

  const char* ac_instr_name;           /**< Instruction name. */
  const char* ac_instr_mnemonic;       /**< Instruction mnemonic. */
  unsigned ac_instr_size;        /**< Instruction size in bytes. */
  unsigned ac_instr_cycles;      /**< Number of cycles of the instruction. */
  unsigned ac_instr_min_latency; /**< Minimum latency in cycles. */
  unsigned ac_instr_max_latency; /**< Maximum latency in cycles. */
  
  /// Default constructor.
  explicit ac_instr_info(int number,
			 const char* name,
			 const char* mnemonic,
			 unsigned size = 0,
			 unsigned cycles = 0,
			 unsigned min_latency = 0,
			 unsigned max_latency = 0) :

    ac_instr_number(number),
    ac_instr_name(name),
    ac_instr_mnemonic(mnemonic),
    ac_instr_size(size),
    ac_instr_cycles(cycles),
    ac_instr_min_latency(min_latency),
    ac_instr_max_latency(max_latency) {};

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INSTR_INFO_H_
