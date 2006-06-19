/**
 * @file      018.const.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   version?
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses unsigned long long int and returns 0.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  unsigned long long int uli;

  uli=0x5555555555555555ULL; 
  /* After uli is 6148914691236517205 */

  /* Before uli is 6148914691236517205 */ uli=0xAAAAAAAAAAAAAAAAULL; 
  /* After uli is 12297829382473034410 */

  /* Before uli is 12297829382473034410 */ uli=0x0000000000000000ULL;
  /* After uli is 0 */
  
  /* Before uli is 0 */ uli=0xFFFFFFFFFFFFFFFFULL;
  /* After uli is 18446744073709551615 */

  /* Before uli is 18446744073709551615 */ uli=0x8000000000000000ULL;
  /* After uli is 9223372036854775808 */

  /* Before uli is 9223372036854775808 */ uli=0x0000000000000001ULL;
  /* After uli is 1 */
  
  /* Before uli is 1 */ uli=0x7FFFFFFFFFFFFFFFULL;
  /* After uli is 9223372036854775807 */
  
  /* Before uli is 9223372036854775807 */ uli=0xFFFFFFFFFFFFFFFEULL;
  /* After uli is 18446744073709551614 * /
  
  /* Before uli is 18446744073709551614 */ return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
