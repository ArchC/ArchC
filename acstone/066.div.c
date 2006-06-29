/**
 * @file      066.div.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses unsigned int division.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned int a,b,c;
  
  a=0xFFFFFFFF;
  b=0x00000001;
  c=a/b;
  /* Before c must be 4294967295 */ c=0;
  
  a=0x0000000F;
  b=0x00000003;
  c=a/b;
  /* Before c must be 5 */ c=0;

  a=0xAAAAAAAA;
  b=0x55555555;
  c=a/b;
  /* Before c must be 2 */ c=0;
  
  a=0x7FFFFFFF;
  b=0xFFFFFFFF;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x00000000;
  b=0x80000000;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x00000030;
  b=0x00000005;
  c=a/b;
  /* Before c must be 9 */ c=0;

  a=0xFFFFFFFF;
  b=0xFFFFFFFF;
  c=a/b;
  /* Before c must be 1 */ c=0;

  a=0xFFFFFFFE;
  b=0x00000010;
  c=a/b;
  /* Before c must be 268435455 */ c=0;

  a=0x00000020;
  b=0xFFFFFFFE;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x00000000;
  b=0xFFFFFFFF;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x78;
  c=a/5;
  c=c/4;
  c=c/3;
  c=c/2;
  /* Before c must be 1 */ c=0;
  
  a=0x10000000;
  c=a/0xFFFFFFFF;
  /* Before c must be 0 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
