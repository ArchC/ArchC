/**
 * @file      gettod.c
 * @author    Marcus Bartholomeu
 *
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:20 -0300
 *
 * @brief     Weak version of gettimeofday function (based in time()).
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

#include <sys/time.h>
#include <errno.h>

int
gettimeofday( struct timeval  *ptimeval,
	      struct timezone *ptimezone)
{
  int t = time(0);
  ptimeval->tv_sec = t;
  ptimeval->tv_usec = 0;
  return 0;
}
