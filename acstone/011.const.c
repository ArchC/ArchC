/**
 * @file      011.const.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed char and returns 0.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  signed char c; 
  
  c=0x55;
  /* After c is 85 */
  
  /* Before c is 85 */ c=0xAA;
  /* After c is -86 */
  
  /* Before c is -86 */ c=0x00;
  /* After c is 0 */
  
  /* Before c is 0 */ c=0xFF; 
  /* After c is -1   */
  
  /* Before c is -1 */ c=0x80; 
  /* After c is -128 */
  
  /* Before c is -128 */ c=0x01; 
  /* After c is 1 */
  
  /* Before c is 1 */ c=0x7F; 
  /* After c is 127 */
  
  /* Before c is 127 */ c=0xFE; 
  /* After c is -2 */
  
  /* Before c is -2 */ return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
