/**
 * @file      068.div.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   version?
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses unsigned long long int division.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned long long int a,b,c;
  
  a=0xFFFFFFFFFFFFFFFFULL;
  b=0x0000000000000001ULL;
  c=a/b;
  /* Before c must be 18446744073709551615 */ c=0;
  
  a=0x000000000000000FULL;
  b=0x0000000000000003ULL;
  c=a/b;
  /* Before c must be 5 */ c=0;

  a=0xAAAAAAAAAAAAAAAAULL;
  b=0x5555555555555555ULL;
  c=a/b;
  /* Before c must be 2 */ c=0;
  
  a=0x7FFFFFFFFFFFFFFFULL;
  b=0xFFFFFFFFFFFFFFFFULL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000000ULL;
  b=0x8000000000000000ULL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000030ULL;
  b=0x0000000000000005ULL;
  c=a/b;
  /* Before c must be 9 */ c=0;

  a=0xFFFFFFFFFFFFFFFFULL;
  b=0xFFFFFFFFFFFFFFFFULL;
  c=a/b;
  /* Before c must be 1 */ c=0;

  a=0xFFFFFFFFFFFFFFFEULL;
  b=0x0000000000000010ULL;
  c=a/b;
  /* Before c must be 1152921504606846975 */ c=0;

  a=0x0000000000000020ULL;
  b=0xFFFFFFFFFFFFFFFEULL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0000000000000000ULL;
  b=0xFFFFFFFFFFFFFFFFULL;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x78;
  c=a/5;
  c=c/4;
  c=c/3;
  c=c/2;
  /* Before c must be 1 */ c=0;
  
  a=0x1000000000000000ULL;
  c=a/0xFFFFFFFFFFFFFFFFULL;
  /* Before c must be 0 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
