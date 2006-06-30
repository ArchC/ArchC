/**
 * @file      021.cast.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main that uses cast signed char to signed short int.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed short int si;
  signed char c;

  si=0;
  c=0x01;
  si=c;

  /* Before si must be 1 */ si=0;
  c=0xFF;
  si=c;
  
  /* Before si must be -1 */ si=0;
  c=0x80;
  si=c;

  /* Before si must be -128 */ si=0;
  c=0x7F;
  si=c;

  /* Before si must be 127 */ si=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
