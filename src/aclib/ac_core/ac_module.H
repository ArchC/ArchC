/**
 * @file   ac_module.H
 * @author Thiago Massariolli Sigrist
 * @date   Thu May 12 10:15:27 2005
 * 
 * @brief  Defines the abstract class for an ArchC module.
 * 
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

  int ac_stop_flag;
  int ac_exit_status;

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

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_MODULE_H_
