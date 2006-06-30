/**
 * @file      061.div.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed char division.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  signed char a,b,c;
  
  a=0xFF;
  b=0x01;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x0F;
  b=0x03;
  c=a/b;
  /* Before c must be 5 */ c=0;

  a=0xAA;
  b=0x55;
  c=a/b;
  /* Before c must be -1 */ c=0;
  
  a=0x7F;
  b=0xFF;
  c=a/b;
  /* Before c must be -127 */ c=0;

  a=0x00;
  b=0x80;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x30;
  b=0x05;
  c=a/b;
  /* Before c must be 9 */ c=0;

  a=0xFF;
  b=0xFF;
  c=a/b;
  /* Before c must be 1 */ c=0;

  a=0xFE;
  b=0x10;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x20;
  b=0xFE;
  c=a/b;
  /* Before c must be -16 */ c=0;

  a=0x00;
  b=0xFF;
  c=a/b;
  /* Before c must be 0 */ c=0;

  a=0x78;
  c=a/5;
  c=c/4;
  c=c/3;
  c=c/2;
  /* Before c must be 1 */ c=0;
  
  a=0x10;
  c=a/(-1);
  /* Before c must be -16 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
