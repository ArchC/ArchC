/**
 * @file      051.mul.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses signed char multiplication.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed char a,b;
  signed short int c;
  
  a=0x02;
  b=0x04;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be 8 */ c=0;

  a=0xFF;
  b=0xFF;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be 1 */ c=0;
  
  a=0xFF;
  b=0x01;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be -1 */ c=0;
  
  a=0xFF;
  b=0x0A;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be -10 */ c=0;

  a=0xFE;
  b=0xFC;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be 8 */ c=0;

  a=0x0A;
  b=0x0B;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be 110 */ c=0;

  a=0xF6;
  b=0x0B;
  c=(signed short int)(a)*(signed short int)(b);
  /* Before c must be -110 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
