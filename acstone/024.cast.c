/**
 * @file      024.cast.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main that uses cast signed short int to signed int.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed int i;
  signed short int si;

  i=0;
  si=0x0001;
  i=si;

  /* Before i must be 1 */ i=0;
  si=0xFFFF;
  i=si;
  
  /* Before i must be -1 */ i=0;
  si=0x8000;
  i=si;

  /* Before i must be -32768 */ i=0;
  si=0x7FFF;
  i=si;

  /* Before i must be 32767 */ i=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
