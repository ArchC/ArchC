/**
 * @file      067.div.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed long long int division.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  signed long long int a,b,c;
  
  a=0xFFFFFFFFFFFFFFFFLL;
  b=0x0000000000000001LL;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x000000000000000FLL;
  b=0x0000000000000003LL;
  c=a/b;
  /* Before c must be 5 */ c=0;

  a=0xAAAAAAAAAAAAAAAALL;
  b=0x5555555555555555LL;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x7FFFFFFFFFFFFFFFLL;
  b=0xFFFFFFFFFFFFFFFFLL;
  c=a/b;
  /* Before c must be -9223372036854775807 */ c=0;

  a=0x0000000000000000LL;
  b=0x8000000000000000LL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000030LL;
  b=0x0000000000000005LL;
  c=a/b;
  /* Before c must be 9 */ c=0;

  a=0xFFFFFFFFFFFFFFFFLL;
  b=0xFFFFFFFFFFFFFFFFLL;
  c=a/b;
  /* Before c must be 1 */ c=0;

  a=0xFFFFFFFFFFFFFFFELL;
  b=0x0000000000000010LL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000020LL;
  b=0xFFFFFFFFFFFFFFFELL;
  c=a/b;
  /* Before c must be -16 */ c=0;

  a=0x0000000000000000LL;
  b=0xFFFFFFFFFFFFFFFFLL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000078LL;
  c=a/5;
  c=c/4;
  c=c/3;
  c=c/2;
  /* Before c must be 1 */ c=0;
  
  a=0x1000000000000000LL;
  c=a/(-1);
  /* Before c must be -1152921504606846976 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
