/**
 * @file      052.mul.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses unsigned char multiplication.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned char a,b;
  unsigned short int c;
  
  a=0x02;
  b=0x04;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 8 */ c=0;

  a=0xFF;
  b=0xFF;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 65025 */ c=0;
  
  a=0xFF;
  b=0x01;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 255 */ c=0;
  
  a=0xFF;
  b=0x0A;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 2550 */ c=0;

  a=0xFE;
  b=0xFC;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 64008 */ c=0;

  a=0x0A;
  b=0x0B;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 110 */ c=0;

  a=0xF6;
  b=0x0B;
  c=(unsigned short int)(a)*(unsigned short int)(b);
  /* Before c must be 2706 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
