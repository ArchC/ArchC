/**
 * @file      kill.c
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
 * @note      The kill isn't implemented yet; go out via exit if self
 *
 */


int
kill(int pid, int sig)
{
  if(pid == 1)
    _exit(sig);
  return 0;
}
