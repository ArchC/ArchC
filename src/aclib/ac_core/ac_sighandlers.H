#ifndef AC_SIGHANDLERS_H
#define AC_SIGHANDLERS_H
//Signal handlers for interrupt and segmentation fault (set in init())
#include <signal.h>
typedef void (*sighandler_t)(int);

void sigint_handler(int signal);
void sigsegv_handler(int signal);
void sigusr1_handler(int signal);
#ifdef USE_GDB
void sigusr2_handler(int signal);
#endif /* USE_GDB */
#endif /* AC_SIGHANDLERS_H */

