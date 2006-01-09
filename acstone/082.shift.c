/**********************************************************************
 It is a simple main function that uses signed and unsigned short int 
 shifts.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned short int usi;
  signed short int ssi;
  
  
  usi=0xFFFF;
  usi=usi>>1;
  /* Before usi must be 32767 */ usi=0;

  usi=0x0000;
  usi=usi>>1;
  /* Before usi must be 0 */ usi=0;

  usi=0x0001;
  usi=usi>>1;
  /* Before usi must be 0 */ usi=0;

  usi=0x8000;
  usi=usi>>1;
  /* Before usi must be 16384 */ usi=0;
  
  usi=0xAAAA;
  usi=usi>>1;
  /* Before usi must be 21845 */ usi=0;
  
  usi=0x03C0;
  usi=usi>>2;
  /* Before usi must be 240 */ usi=0;

  
  ssi=0xFFFF;
  ssi=ssi>>1;
  /* Before ssi must be -1 */ ssi=0;

  ssi=0x0000;
  ssi=ssi>>1;
  /* Before ssi must be 0 */ ssi=0;

  ssi=0x0001;
  ssi=ssi>>1;
  /* Before ssi must be 0 */ ssi=0;

  ssi=0x8000;
  ssi=ssi>>1;
  /* Before ssi must be -16384 */ ssi=0;
  
  ssi=0xAAAA;
  ssi=ssi>>1;
  /* Before ssi must be -10923 */ ssi=0;
  
  ssi=0x03C0;
  ssi=ssi>>2;
  /* Before ssi must be 240 */ ssi=0;

    
  usi=0xFFFF;
  usi=usi<<1;
  /* Before usi must be 65534 */ usi=0;

  usi=0x0000;
  usi=usi<<1;
  /* Before usi must be 0 */ usi=0;

  usi=0x0001;
  usi=usi<<1;
  /* Before usi must be 2 */ usi=0;

  usi=0x8000;
  usi=usi<<1;
  /* Before usi must be 0 */ usi=0;
  
  usi=0xAAAA;
  usi=usi<<1;
  /* Before usi must be 21844 */ usi=0;
  
  usi=0x03C0;
  usi=usi<<2;
  /* Before usi must be 3840 */ usi=0;


  ssi=0xFFFF;
  ssi=ssi<<1;
  /* Before ssi must be -2 */ ssi=0;

  ssi=0x0000;
  ssi=ssi<<1;
  /* Before ssi must be 0 */ ssi=0;

  ssi=0x0001;
  ssi=ssi<<1;
  /* Before ssi must be 2 */ ssi=0;

  ssi=0x8000;
  ssi=ssi<<1;
  /* Before ssi must be 0 */ ssi=0;
  
  ssi=0xAAAA;
  ssi=ssi<<1;
  /* Before ssi must be 21844 */ ssi=0;
  
  ssi=0x03C0;
  ssi=ssi<<2;
  /* Before ssi must be 3840 */ ssi=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
