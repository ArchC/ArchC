/**********************************************************************
 It is a simple main that uses cast signed int to signed long long int.
**********************************************************************/

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
