/* kill -- not implemented yet */
/* go out via exit if self */

int
kill(int pid, int sig)
{
  if(pid == 1)
    _exit(sig);
  return 0;
}
