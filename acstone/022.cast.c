/**
 * @file      022.cast.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main that uses cast signed char to signed int.
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
  signed char c;

  i=0;
  c=0x01;
  i=c;

  /* Before i must be 1 */ i=0;
  c=0xFF;
  i=c;
  
  /* Before i must be -1 */ i=0;
  c=0x80;
  i=c;

  /* Before i must be -128 */ i=0;
  c=0x7F;
  i=c;

  /* Before i must be 127 */ i=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
