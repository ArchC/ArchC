/**
 * @file      053.mul.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses signed short int multiplication.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed short int a,b;
  signed int c;
  
  a=0x0002;
  b=0x0004;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 8 */ c=0;

  a=0xFFFF;
  b=0xFFFF;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 1 */ c=0;
  
  a=0xFFFF;
  b=0x0001;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be -1 */ c=0;
  
  a=0xFFFF;
  b=0x000A;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be -10 */ c=0;

  a=0xFFFE;
  b=0xFFFC;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 8 */ c=0;

  a=0x000A;
  b=0x000B;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 110 */ c=0;

  a=0xFFF6;
  b=0x000B;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be -110 */ c=0;

  a=0xF000;
  b=0xF001;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 16773120 */ c=0;

  a=0x5555;
  b=0x3333;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be 286322415 */ c=0;

  a=0x5353;
  b=0x8000;
  c=(signed int)(a)*(signed int)(b);
  /* Before c must be -698974208 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
