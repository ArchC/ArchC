/**********************************************************************
 It is a simple main function that uses while loops.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned long long int cont;

  unsigned char a;
  signed char b;
  unsigned short int c;
  signed short int d;
  unsigned int e;
  signed int f;
  unsigned long long int g;
  signed long long int h;
  
  cont=0;
  
  a=5;
  while(a--)
    cont++;
  /* Before a must be 255 and cont 5 */ a=0;
  
  b=5;
  while(b--)
    cont++;
  /* Before b must be -1 and cont 10 */ b=0;

  c=5;
  while(c--)
    cont++;
  /* Before c must be 65535 and cont 15 */ c=0;

  d=5;
  while(d--)
    cont++;
  /* Before d must be -1 and cont 20 */ d=0;

  e=5;
  while(e--)
    cont++;
  /* Before e must be 4294967295 and cont 25 */ e=0;

  f=5;
  while(f--)
    cont++;
  /* Before f must be -1 and cont 30 */ f=0;

  g=5;
  while(g--)
    cont++;
  /* Before g must be 18446744073709551615 and cont 35 */ g=0;

  h=5;
  while(h--)
    cont++;
  /* Before h must be -1 and cont 40 */ h=0;


  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
