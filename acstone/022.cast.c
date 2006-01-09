/**********************************************************************
 It is a simple main that uses cast signed char to signed int.
**********************************************************************/

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
