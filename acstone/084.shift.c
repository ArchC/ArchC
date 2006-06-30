/**
 * @file      084.shift.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses signed and unsigned long long int shifts.
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
  signed long long int sli;
  
  uli=0xFFFFFFFFFFFFFFFFULL;
  uli=uli>>1;
  /* Before uli must be 9223372036854775807 */ uli=0;

  uli=0x0000000000000000ULL;
  uli=uli>>1;
  /* Before uli must be 0 */ uli=0;

  uli=0x0000000000000001ULL;
  uli=uli>>1;
  /* Before uli must be 0 */ uli=0;

  uli=0x8000000000000000ULL;
  uli=uli>>1;
  /* Before uli must be 4611686018427387904 */ uli=0;
  
  uli=0xAAAAAAAAAAAAAAAAULL;
  uli=uli>>1;
  /* Before uli must be 6148914691236517205 */ uli=0;
  
  uli=0x00000003C0000000ULL;
  uli=uli>>2;
  /* Before uli must be 4026531840 */ uli=0;

  
  sli=0xFFFFFFFFFFFFFFFFLL;
  sli=sli>>1;
  /* Before sli must be -1 */ sli=0;

  sli=0x0000000000000000LL;
  sli=sli>>1;
  /* Before sli must be 0 */ sli=0;

  sli=0x0000000000000001LL;
  sli=sli>>1;
  /* Before sli must be 0 */ sli=0;

  sli=0x8000000000000000LL;
  sli=sli>>1;
  /* Before sli must be -4611686018427387904 */ sli=0;
  
  sli=0xAAAAAAAAAAAAAAAALL;
  sli=sli>>1;
  /* Before sli must be -3074457345618258603 */ sli=0;
  
  sli=0x00000003C0000000LL;
  sli=sli>>2;
  /* Before sli must be 4026531840 */ sli=0;

    
  uli=0xFFFFFFFFFFFFFFFFULL;
  uli=uli<<1;
  /* Before uli must be 18446744073709551614 */ uli=0;

  uli=0x0000000000000000ULL;
  uli=uli<<1;
  /* Before uli must be 0 */ uli=0;

  uli=0x0000000000000001ULL;
  uli=uli<<1;
  /* Before uli must be 2 */ uli=0;

  uli=0x8000000000000000ULL;
  uli=uli<<1;
  /* Before uli must be 0 */ uli=0;
  
  uli=0xAAAAAAAAAAAAAAAAULL;
  uli=uli<<1;
  /* Before uli must be 6148914691236517204 */ uli=0;
  
  uli=0x00000003C0000000ULL;
  uli=uli<<2;
  /* Before uli must be 64424509440 */ uli=0;


  sli=0xFFFFFFFFFFFFFFFFLL;
  sli=sli<<1;
  /* Before sli must be -2 */ sli=0;

  sli=0x0000000000000000LL;
  sli=sli<<1;
  /* Before sli must be 0 */ sli=0;

  sli=0x0000000000000001LL;
  sli=sli<<1;
  /* Before sli must be 2 */ sli=0;

  sli=0x8000000000000000LL;
  sli=sli<<1;
  /* Before sli must be 0 */ sli=0;
  
  sli=0xAAAAAAAAAAAAAAAALL;
  sli=sli<<1;
  /* Before sli must be 6148914691236517204 */ sli=0;
  
  sli=0x00000003C0000000LL;
  sli=sli<<2;
  /* Before sli must be 64424509440 */ sli=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
