/**
 * @file      ac_module.H
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
 * @date      Thu May 12 10:15:27 2005
 * 
 * @brief     Defines the abstract class for an ArchC module.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_MODULE_H_
#define _AC_MODULE_H_

//////////////////////////////////////////////////////////////////////////////

// Standard includes
#include <list>

// SystemC includes
#include <systemc.h>

// ArchC includes

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

/// Abstract class for an ArchC processor/simulator module.
class ac_module: public sc_module
{
 private:
  /// The next module ID.
  static unsigned next_mod_id;

  /// Number of currently running ac_modules.
  static unsigned running_mods;

  /// List of all modules.
  static std::list<ac_module*> mods_list;

  /// Pointer to self in the list.
  std::list<ac_module*>::iterator this_mod;

 public:
  /// Module unique ID.
  const unsigned mod_id;

  int ac_exit_status;

  unsigned int instr_in_batch;
  unsigned int instr_batch_size;

  // SystemC special declaration.
  SC_HAS_PROCESS(ac_module);

  /// Standard constructor.
  ac_module();

  /// Named constructor.
  ac_module(sc_module_name nm);

  /// Destructor.
  virtual ~ac_module();

  /// PrintStat placeholder.
  virtual void PrintStat();

  /// Callable PrintStat-like method.
  static void PrintAllStats();

  /// Public method that registers module as a running module.
  void set_running();

  /// Public method that unregisters module (ie, it's no longer running).
  void set_stopped();

  /// Public method that sets the size of the uninterrupted instruction batch
  void set_instr_batch_size(unsigned int size);

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_MODULE_H_
