/**
 * @file      ac_sighandlers.cpp
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
 * @brief     
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include "ac_sighandlers.H"
#include <stdlib.h>
#include "ac_module.H"

void sigint_handler(int signal)
{
  fprintf(stderr, "ArchC: INTERUPTED BY THE SIGNAL %d\n", signal);
  ac_module::PrintAllStats();
  exit(EXIT_FAILURE);
}
void sigsegv_handler(int signal)
{
  fprintf(stderr, "ArchC Error: Segmentation fault.\n");
  ac_module::PrintAllStats();
  exit(EXIT_FAILURE);
}
void sigusr1_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Printing statistics\n", signal);
  ac_module::PrintAllStats();
  fprintf(stderr, "ArchC: -------------------- Continuing Simulation ------------------\n");
}

void sigusr2_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Starting GDB support (not implemented).\n", signal);
/*  gdbstub->enable();
  gdbstub->connect();*/
}

