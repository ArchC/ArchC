/**
 * @file      ac_sighandlers.H
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
 * @brief     Signal handlers for interrupt and segmentation fault (set in init())
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#ifndef AC_SIGHANDLERS_H
#define AC_SIGHANDLERS_H
#include <signal.h>
typedef void (*sighandler_t)(int);

void sigint_handler(int signal);
void sigsegv_handler(int signal);
void sigusr1_handler(int signal);
#ifdef USE_GDB
void sigusr2_handler(int signal);
#endif /* USE_GDB */
#endif /* AC_SIGHANDLERS_H */

