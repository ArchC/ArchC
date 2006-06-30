/**
 * @file      026.cast.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main that uses cast signed int to signed long long int.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed long long int li;
  signed int i;

  li=0;
  i=0x00000001;
  li=i;

  /* Before li must be 1 */ li=0;
  i=0xFFFFFFFF;
  li=i;
  
  /* Before li must be -1 */ li=0;
  i=0x80000000;
  li=i;

  /* Before li must be -2147483648 */ li=0;
  i=0x7FFFFFFF;
  li=i;

  /* Before li must be 2147483647 */ li=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
