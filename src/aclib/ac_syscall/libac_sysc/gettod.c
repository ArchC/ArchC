/******************************************************************

  File:	 gettimeofday.c
  Description: Weak version of gettimeofday function (based in time()).
  ID: 22/08/2003 - Bartho
  Modifications:
	
******************************************************************/
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
