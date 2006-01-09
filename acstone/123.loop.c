/**********************************************************************
 It is a simple main function that uses a implemented unsigned
 multiplication 
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned char count;
  unsigned int C,A,Q,M;
  unsigned long long int result;

  count=32; /* 32 bits * 32 bits = 64 bits */

  /* Some variable */
  M=0x1314AB42;
  Q=0xF1B34517;
  A=0x00000000;
  C=0x00000000;

  while(count!=0) {
    
    /* if q0 = 1 add */
    if(Q & 0x00000001) {
      if(((unsigned long long int)A+
	  (unsigned long long int)M) > 0xFFFFFFFF) /* Check if have carry */
	C=1;
      else
	C=0;
      A=A+M;
    }

    /* Shift */
    Q=Q>>1;
    if(A & 0x00000001)
      Q=(Q | 0x80000000);
    A=A>>1;
    if(C & 0x00000001)
      A=(A | 0x80000000);
    C=C>>1;

    /* Decrement counter */    
    count--;

  }
  
  result=0;
  result=((((unsigned long long int)(A)) << 32) | 
	  ((unsigned long long int)(Q)));
  /* Before result must be 1298111822488546542 */ result=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
