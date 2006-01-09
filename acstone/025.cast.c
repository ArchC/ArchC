/**********************************************************************
 It is a simple main that uses cast signed short int to signed long 
 long int.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed long long int li;
  signed short int si;

  li=0;
  si=0x0001;
  li=si;

  /* Before li must be 1 */ li=0;
  si=0xFFFF;
  li=si;
  
  /* Before li must be -1 */ li=0;
  si=0x8000;
  li=si;

  /* Before li must be -32768 */ li=0;
  si=0x7FFF;
  li=si;

  /* Before li must be 32767 */ li=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
