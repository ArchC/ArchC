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
