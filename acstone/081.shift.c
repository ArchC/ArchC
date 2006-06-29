/**
 * @file      081.shift.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed and unsigned char shifts.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned char uc;
  signed char sc;
  
  
  uc=0xFF;
  uc=uc>>1;
  /* Before uc must be 127 */ uc=0;

  uc=0x00;
  uc=uc>>1;
  /* Before uc must be 0 */ uc=0;

  uc=0x01;
  uc=uc>>1;
  /* Before uc must be 0 */ uc=0;

  uc=0x80;
  uc=uc>>1;
  /* Before uc must be 64 */ uc=0;
  
  uc=0xAA;
  uc=uc>>1;
  /* Before uc must be 85 */ uc=0;
  
  uc=0x3C;
  uc=uc>>2;
  /* Before uc must be 15 */ uc=0;

  
  sc=0xFF;
  sc=sc>>1;
  /* Before sc must be -1 */ sc=0;

  sc=0x00;
  sc=sc>>1;
  /* Before sc must be 0 */ sc=0;

  sc=0x01;
  sc=sc>>1;
  /* Before sc must be 0 */ sc=0;

  sc=0x80;
  sc=sc>>1;
  /* Before sc must be -64 */ sc=0;
  
  sc=0xAA;
  sc=sc>>1;
  /* Before sc must be -43 */ sc=0;
  
  sc=0x3C;
  sc=sc>>2;
  /* Before sc must be 15 */ sc=0;

    
  uc=0xFF;
  uc=uc<<1;
  /* Before uc must be 254 */ uc=0;

  uc=0x00;
  uc=uc<<1;
  /* Before uc must be 0 */ uc=0;

  uc=0x01;
  uc=uc<<1;
  /* Before uc must be 2 */ uc=0;

  uc=0x80;
  uc=uc<<1;
  /* Before uc must be 0 */ uc=0;
  
  uc=0xAA;
  uc=uc<<1;
  /* Before uc must be 84 */ uc=0;
  
  uc=0x3C;
  uc=uc<<2;
  /* Before uc must be 240 */ uc=0;


  sc=0xFF;
  sc=sc<<1;
  /* Before sc must be -2 */ sc=0;

  sc=0x00;
  sc=sc<<1;
  /* Before sc must be 0 */ sc=0;

  sc=0x01;
  sc=sc<<1;
  /* Before sc must be 2 */ sc=0;

  sc=0x80;
  sc=sc<<1;
  /* Before sc must be 0 */ sc=0;
  
  sc=0xAA;
  sc=sc<<1;
  /* Before sc must be 84 */ sc=0;
  
  sc=0x3C;
  sc=sc<<2;
  /* Before sc must be -16 */ sc=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
