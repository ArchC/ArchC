/**
 * @file      ac_arch.H
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 * 
 * @brief     ArchC Resources header file.
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef  _AC_ARCH_H_
#define  _AC_ARCH_H_

#include <sys/times.h>
#include <stdio.h>

#include  "ac_regbank.H"
#include  "ac_rtld.H"

template <typename T, typename U> class ac_memport;

#ifdef USE_GDB
template <typename ac_word> class AC_GDB;
#endif // USE_GDB

///ArchC class for Architecture Resources.

template <typename ac_word, typename ac_Hword> class ac_arch {
private:
  typedef change_log<ac_word> chg_log;
  typedef list<chg_log> log_list;

public:
  /// Indicates the storage device from where instructions are fetched.
  ac_memport<ac_word, ac_Hword>* IM;

  /// Indicates the storage device where applications are loaded.
  ac_memport<ac_word, ac_Hword>* APP_MEM;

  /// Control Variables.
  bool ac_wait_sig;
  bool ac_parallel_sig;
  bool ac_annul_sig;
  bool ac_mt_endian;
  bool ac_tgt_endian;
  unsigned ac_start_addr;
  unsigned long long ac_instr_counter;
  unsigned long long ac_cycle_counter;
  int ac_stop_flag;
  double time_step;
  int argc;
  char **argv;

  // Timing structures.
  struct tms ac_run_times;
  clock_t ac_run_start_time;

  /// Heap pointer.
  unsigned int ac_heap_ptr;

  /// Decoder cache size.
  unsigned dec_cache_size;

  /// Decoder buffer.
  ac_word* buffer;

  /// Dynamic linker and loader
  ac_dynlink::ac_rtld ac_dyn_loader;

  /// Decoder variables.
  unsigned int quant, decode_pc;

  /// Constructor.
  explicit ac_arch(int max_buffer) :
    ac_wait_sig(0),
    ac_parallel_sig(0),
    ac_annul_sig(0),
    ac_mt_endian(0),
    ac_tgt_endian(0),
    ac_start_addr(0),
    ac_instr_counter(0),
    ac_cycle_counter(0),
    ac_stop_flag(0),
    ac_heap_ptr(0),
    dec_cache_size(0),
    quant(0),
    decode_pc(0) {

    buffer = new ac_word[max_buffer];

  }

  /// Initializes program arguments.
  void set_args( int ac, char **av){
    argc = ac;
    argv = av;
  };

#ifdef AC_DELAY 
  /// Delayed Assignment.
  chg_log delay(ac_word value, unsigned clocks){
    return chg_log(0, value, clocks + ac_cycle_counter );
  };
#endif // AC_DELAY

  /// Stall method.
  /// Puts the simulator on the wait state.
  void ac_wait(){
    ac_wait_sig = 1;
  };

  /// Releases the simulator from the wait state.
  void ac_release(){
    ac_wait_sig = 0;
  };

  /// Annulates the current instruction.
  void ac_annul(){
    ac_annul_sig = 1;
  };

  /// Forces Paralelism.
  void ac_parallel( ){
    ac_parallel_sig = 1;
  };

  void InitStat() {
    ac_run_start_time = times(&ac_run_times);
  }

  void PrintStat() {
    clock_t ac_run_real;

    //Print statistics
    fprintf(stderr, "ArchC: Simulation statistics\n");

    ac_run_real = times(&ac_run_times) - ac_run_start_time;
    fprintf(stderr, "    Times: %ld.%02ld user, %ld.%02ld system, %ld.%02ld real\n",
            ac_run_times.tms_utime / 100, ac_run_times.tms_utime % 100,
            ac_run_times.tms_stime / 100, ac_run_times.tms_stime % 100,
            ac_run_real / 100, ac_run_real % 100
           );

    fprintf(stderr, "    Number of instructions executed: %llu\n", ac_instr_counter);

    if (ac_run_times.tms_utime > 5) {
      double ac_mips = (ac_instr_counter * 100) / ac_run_times.tms_utime;
      fprintf(stderr, "    Simulation speed: %.2f K instr/s\n", ac_mips/1000);
    }
    else {
      fprintf(stderr, "    Simulation speed: (too fast to be precise)\n");
    }
  }

  virtual void init() = 0;

  virtual void init(int ac, char *av[]) = 0;

  virtual void stop(int status = 0) = 0;

  /**
   * Loads program into the processor instruction memory during elaboration.
   * @param program Path of the binary file containing the program.
   */
  virtual void load(char* program) = 0;

  /**
   * Loads program into the processor instruction memory during simulation.
   * @param program Path of the binary file containing the program.
   */
  virtual void delayed_load(char* program) = 0;

#ifdef USE_GDB
  virtual AC_GDB<ac_word>* get_gdbstub() = 0;
#endif // USE_GDB

  virtual ~ac_arch() {
    delete[] buffer;
  };

 // Read access to ac_pc (placeholder).
  virtual unsigned get_ac_pc() = 0;
};

#endif  // _AC_ARCH_H_
