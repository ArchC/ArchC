/**
 * @file   ac_instr_info.H
 * @author Thiago Massariolli Sigrist
 * @date   Thu May 12 13:54:55 2005
 * 
 * @brief  Defines a struct containing information about an instruction.
 * 
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
/// ac_instr_info<isa_class>.
template<class isa_class> struct ac_instr_info {

  /// Parametric type for a processor ISA instruction behavior.
  typedef void (isa_class::*bhv_function)();

  /// Instruction number.
  int ac_instr_number;

  /// Pointer to specific instruction behavior.
  bhv_function ac_instr_behavior;

  /// Pointer to instruction's type behavior.
  bhv_function ac_instr_type_behavior;

  char* ac_instr_name;           /**< Instruction name. */
  char* ac_instr_mnemonic;       /**< Instruction mnemonic. */
  unsigned ac_instr_size;        /**< Instruction size in bytes. */
  unsigned ac_instr_cycles;      /**< Number of cycles of the instruction. */
  unsigned ac_instr_min_latency; /**< Minimum latency in cycles. */
  unsigned ac_instr_max_latency; /**< Maximum latency in cycles. */
  
  /// Default constructor.
  explicit ac_instr_info(int number,
			 bhv_function behavior,
			 bhv_function type_behavior,
			 char* name,
			 char* mnemonic,
			 unsigned size = 0,
			 unsigned cycles = 0,
			 unsigned min_latency = 0,
			 unsigned max_latency = 0) :

    ac_instr_number(number),
    ac_instr_behavior(behavior),
    ac_instr_type_behavior(type_behavior),
    ac_instr_name(name),
    ac_instr_mnemonic(mnemonic),
    ac_instr_size(size),
    ac_instr_cycles(cycles),
    ac_instr_min_latency(min_latency),
    ac_instr_max_latency(max_latency) {};

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_INSTR_INFO_H_
