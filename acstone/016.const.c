/**********************************************************************
 It is a simple main function that uses unsigned int and returns 0.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  unsigned int ui;

  ui=0x55555555; 
  /* After ui is 1431655765 */

  /* Before ui is 1431655765 */ ui=0xAAAAAAAA; 
  /* After ui is 2863311530 */

  /* Before ui is 2863311530 */ ui=0x00000000; 
  /* After ui is 0 */
 
  /* Before ui is 0 */ ui=0xFFFFFFFF;
  /* After ui is 4294967295 */

  /* Before ui is 4294967295 */ ui=0x80000000;
  /* After ui is 2147483648 */

  /* Before ui is 2147483648 */ ui=0x00000001;
  /* After ui is 1 */

  /* Before ui is 1 */ ui=0x7FFFFFFF;
  /* After ui is 2147483647 */
 
  /* Before ui is 2147483647 */ ui=0xFFFFFFFE;
  /* After ui is 4294967294 */
  
  /* Before ui is 4294967294 */ return 0;
  /* Return 0 */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
