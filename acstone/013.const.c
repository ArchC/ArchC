/**
 * @file      013.const.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed short int and returns 0.
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

  si=0x5555; 
  /* After si is 21845 */

  /* Before si is 21845  */ si=0xAAAA; 
  /* After si is -21846 */

  /* Before si is -21846 */ si=0x0000; 
  /* After si is 0 */

  /* Before si is 0 */ si=0xFFFF; 
  /* After si is -1 */

  /* Before si is -1 */ si=0x8000; 
  /* After si is -32768 */

  /* Before si is -32768 */ si=0x0001; 
  /* After si is 1 */

  /* Before si is 1 */ si=0x7FFF; 
  /* After si is 32767 */

  /* Before si is 32767  */ si=0xFFFE; 
  /* After si is -2 */

  /* Before si is -2 */ return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
