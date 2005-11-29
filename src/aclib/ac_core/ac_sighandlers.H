//Signal handlers for interrupt and segmentation fault (set in ac_start() )
#include <signal.h>
typedef void (*sighandler_t)(int);
void sigint_handler(int signal)
{
  fprintf(stderr, "ArchC: INTERUPTED BY THE SIGNAL %d\n", signal);
//   PrintStat();
  exit(EXIT_FAILURE);
}
void sigsegv_handler(int signal)
{
  fprintf(stderr, "ArchC Error: Segmentation fault.\n");
//   PrintStat();
  exit(EXIT_FAILURE);
}
void sigusr1_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Printing statistics\n", signal);
//   PrintStat();
  fprintf(stderr, "ArchC: -------------------- Continuing Simulation ------------------\n");
}

#ifdef USE_GDB
void sigusr2_handler(int signal)
{
  fprintf(stderr, "ArchC: Received signal %d. Starting GDB support\n", signal);
  gdbstub->enable();
  gdbstub->connect();
}
#endif /* USE_GDB */
