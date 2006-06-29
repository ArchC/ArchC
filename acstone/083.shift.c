/**
 * @file      083.shift.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed and unsigned int shifts.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned int ui;
  signed int si;
  
  
  ui=0xFFFFFFFF;
  ui=ui>>1;
  /* Before ui must be 2147483647 */ ui=0;

  ui=0x00000000;
  ui=ui>>1;
  /* Before ui must be 0 */ ui=0;

  ui=0x00000001;
  ui=ui>>1;
  /* Before ui must be 0 */ ui=0;

  ui=0x80000000;
  ui=ui>>1;
  /* Before ui must be 1073741824 */ ui=0;
  
  ui=0xAAAAAAAA;
  ui=ui>>1;
  /* Before ui must be 1431655765 */ ui=0;
  
  ui=0x0003C000;
  ui=ui>>2;
  /* Before ui must be 61440 */ ui=0;

  
  si=0xFFFFFFFF;
  si=si>>1;
  /* Before si must be -1 */ si=0;

  si=0x00000000;
  si=si>>1;
  /* Before si must be 0 */ si=0;

  si=0x00000001;
  si=si>>1;
  /* Before si must be 0 */ si=0;

  si=0x80000000;
  si=si>>1;
  /* Before si must be -1073741824 */ si=0;
  
  si=0xAAAAAAAA;
  si=si>>1;
  /* Before si must be -715827883 */ si=0;
  
  si=0x0003C000;
  si=si>>2;
  /* Before si must be 61440 */ si=0;

    
  ui=0xFFFFFFFF;
  ui=ui<<1;
  /* Before ui must be 4294967294 */ ui=0;

  ui=0x00000000;
  ui=ui<<1;
  /* Before ui must be 0 */ ui=0;

  ui=0x00000001;
  ui=ui<<1;
  /* Before ui must be 2 */ ui=0;

  ui=0x80000000;
  ui=ui<<1;
  /* Before ui must be 0 */ ui=0;
  
  ui=0xAAAAAAAA;
  ui=ui<<1;
  /* Before ui must be 1431655764 */ ui=0;
  
  ui=0x0003C000;
  ui=ui<<2;
  /* Before ui must be 983040 */ ui=0;


  si=0xFFFFFFFF;
  si=si<<1;
  /* Before si must be -2 */ si=0;

  si=0x00000000;
  si=si<<1;
  /* Before si must be 0 */ si=0;

  si=0x00000001;
  si=si<<1;
  /* Before si must be 2 */ si=0;

  si=0x80000000;
  si=si<<1;
  /* Before si must be 0 */ si=0;
  
  si=0xAAAAAAAA;
  si=si<<1;
  /* Before si must be 1431655764 */ si=0;
  
  si=0x0003C000;
  si=si<<2;
  /* Before si must be 983040 */ si=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
