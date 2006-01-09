/**********************************************************************
 It is a simple main function that uses signed short int and returns 0.
**********************************************************************/

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
