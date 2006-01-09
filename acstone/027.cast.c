/**********************************************************************
 It is a simple main that uses some unsigned casts.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned long long int uli;
  unsigned int ui;
  unsigned short int usi;
  unsigned char uc;

  uc=0xFF;
  usi=0xFFFF;
  usi=uc;
  /* Before usi must be 225 */ usi=0;
  
  usi=0xFFFF;
  ui=0xFFFFFFFF;
  ui=usi;
  /* Before ui must be 65535 */ ui=0;

  ui=0xFFFFFFFF;
  uli=0xFFFFFFFFFFFFFFFFULL;
  uli=ui;
  /* Before uli must be 4294967295 */ uli=0;

  return 0; 
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
