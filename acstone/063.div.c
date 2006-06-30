/**
 * @file      063.div.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed short int division.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  signed short int a,b,c;
  
  a=0xFFFF;
  b=0x0001;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x000F;
  b=0x0003;
  c=a/b;
  /* Before c must be 5 */ c=0;

  a=0xAAAA;
  b=0x5555;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x7FFF;
  b=0xFFFF;
  c=a/b;
  /* Before c must be -32767 */ c=0;

  a=0x0000;
  b=0x8000;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0030;
  b=0x0005;
  c=a/b;
  /* Before c must be 9 */ c=0;

  a=0xFFFF;
  b=0xFFFF;
  c=a/b;
  /* Before c must be 1 */ c=0;

  a=0xFFFE;
  b=0x0010;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0020;
  b=0xFFFE;
  c=a/b;
  /* Before c must be -16 */ c=0;

  a=0x0000;
  b=0xFFFF;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x0078;
  c=a/5;
  c=c/4;
  c=c/3;
  c=c/2;
  /* Before c must be 1 */ c=0;
  
  a=0x1000;
  c=a/(-1);
  /* Before c must be -4096 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
