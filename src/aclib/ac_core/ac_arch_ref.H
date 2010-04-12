/**
 * @file      ac_arch_ref.H
 * @author    Thiago Massariolli Sigrist
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Thu May 12 14:01:01 2005
 * 
 * @brief     Defines a class that contains references to the ac_arch fields.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_ARCH_REF_H_
#define _AC_ARCH_REF_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <iostream>

// SystemC includes

// ArchC includes
#include "ac_log.H"
#include "ac_arch.H"
#include "ac_rtld.H"

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile
template<class T, class U> class ac_memport;

//////////////////////////////////////////////////////////////////////////////

/// Class containing references to the ac_arch fields.
template <typename ac_word, typename ac_Hword> class ac_arch_ref {
private:
  ac_arch<ac_word, ac_Hword>& archref;
public:

  /// Indicates the storage device from where instructions are fetched.
  ac_memport<ac_word, ac_Hword>*& IM;

  /// Indicates the storage device where applications are loaded.
  ac_memport<ac_word, ac_Hword>*& APP_MEM;

  // Control Variables.
  bool& ac_wait_sig;
  bool& ac_parallel_sig;
  bool& ac_annul_sig;
  bool& ac_mt_endian;
  bool& ac_tgt_endian;
  unsigned& ac_start_addr;
  ac_dynlink::ac_rtld& ac_dyn_loader;
  unsigned long long& ac_instr_counter;
  unsigned long long& ac_cycle_counter;
  int& ac_stop_flag;
  double& time_step;
  int& argc;
  char**& argv;

  /// Heap pointer.
  unsigned int& ac_heap_ptr;

  /// Decoder cache size.
  unsigned& dec_cache_size;

  /// Default constructor
  ac_arch_ref(ac_arch<ac_word, ac_Hword>& arch) :
    archref(arch),
    IM(arch.IM),
    APP_MEM(arch.APP_MEM),
    ac_wait_sig(arch.ac_wait_sig),
    ac_parallel_sig(arch.ac_parallel_sig),
    ac_annul_sig(arch.ac_annul_sig),
    ac_mt_endian(arch.ac_mt_endian),
    ac_tgt_endian(arch.ac_tgt_endian),
    ac_start_addr(arch.ac_start_addr),
    ac_dyn_loader(arch.ac_dyn_loader),
    ac_instr_counter(arch.ac_instr_counter),
    ac_cycle_counter(arch.ac_cycle_counter),
    ac_stop_flag(arch.ac_stop_flag),
    time_step(arch.time_step),
    argc(arch.argc),
    argv(arch.argv),
    ac_heap_ptr(arch.ac_heap_ptr),
    dec_cache_size(arch.dec_cache_size) {}

  /// Initializing program arguments.
  void set_args(int ac, char **av) {
    argc = ac;
    argv = av;
  }

#ifdef AC_DELAY
  /// Delayed Assignment.
  change_log<ac_word> delay(ac_word val, unsigned clocks) {
    return change_log<ac_word>(0, val, clocks + ac_cycle_counter);
  }
#endif // AC_DELAY

  /// Stall method.
  /// Put the simulator on the wait state.
  void ac_wait() {
    ac_wait_sig = 1;
  }

  /// Release the simulator from the wait state.
  void ac_release() {
    ac_wait_sig = 0;
  }

  /// Annul the current instruction.
  void ac_annul() {
    ac_annul_sig = 1;
  }

  /// Force Paralelism.
  void ac_parallel() {
    ac_parallel_sig = 1;
  }

  /// Stop method.
  void stop(int status = 0)
  {
   archref.stop(status);
   return;
  }

  /// Read access to ac_pc (placeholder).
  virtual unsigned get_ac_pc()
  {
   return archref.get_ac_pc();
  }
};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_ARCH_REF_H_
