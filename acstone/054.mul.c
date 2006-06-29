/**
 * @file      054.mul.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses unsigned short int multiplication.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */
 
/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned short int a,b;
  unsigned int c;
  
  a=0x0002;
  b=0x0004;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 8 */ c=0;

  a=0xFFFF;
  b=0xFFFF;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 4294836225 */ c=0;
  
  a=0xFFFF;
  b=0x0001;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 65535 */ c=0;
  
  a=0xFFFF;
  b=0x000A;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 655350 */ c=0;

  a=0xFFFE;
  b=0xFFFC;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 4294574088 */ c=0;

  a=0x000A;
  b=0x000B;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 110 */ c=0;

  a=0xFFF6;
  b=0x000B;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 720786 */ c=0;

  a=0xF000;
  b=0xF001;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 3774935040 */ c=0;

  a=0x5555;
  b=0x3333;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 286322415 */ c=0;

  a=0x5353;
  b=0x8000;
  c=(unsigned int)(a)*(unsigned int)(b);
  /* Before c must be 698974208 */ c=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
