/**********************************************************************
 It is a simple main function that uses int boolean operators.
**********************************************************************/

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned int a,b,c,d,e,f,g,h,i,j;
  
  a=0xFFFFFFFF;
  b=0xFFFFFFFF;
  c=a|b;
  /* Before c must be 4294967295 */ c=0;
  d=a&b;
  /* Before d must be 4294967295 */ d=0;
  e=a^b;
  /* Before e must be 0 */ e=0;
  f=~(a|b);
  /* Before f must be 0 */ f=0;
  g=~(a&b);
  /* Before g must be 0 */ g=0;
  h=~(a^b);
  /* Before h must be 4294967295 */ h=0;
  i=~a;
  /* Before i must be 0 */ i=0;
  j=~b;
  /* Before j must be 0 */ j=0;

  a=0x0F0F0F0F;
  b=0xF0F0F0F0;
  c=a|b;
  /* Before c must be 4294967295 */ c=0;
  d=a&b;
  /* Before d must be 0 */ d=0;
  e=a^b;
  /* Before e must be 4294967295 */ e=0;
  f=~(a|b);
  /* Before f must be 0 */ f=0;
  g=~(a&b);
  /* Before g must be 4294967295 */ g=0;
  h=~(a^b);
  /* Before h must be 0 */ h=0;
  i=~a;
  /* Before i must be 4042322160 */ i=0;
  j=~b;
  /* Before j must be 252645135 */ j=0;

  a=0xFFFFFFFF;
  b=0x00000000;
  c=a|b;
  /* Before c must be 4294967295 */ c=0;
  d=a&b;
  /* Before d must be 0 */ d=0;
  e=a^b;
  /* Before e must be 4294967295 */ e=0;
  f=~(a|b);
  /* Before f must be 0 */ f=0;
  g=~(a&b);
  /* Before g must be 4294967295 */ g=0;
  h=~(a^b);
  /* Before h must be 0 */ h=0;
  i=~a;
  /* Before i must be 0 */ i=0;
  j=~b;
  /* Before j must be 4294967295 */ j=0;

  a=0x55555555;
  b=0xAAAAAAAA;
  c=a|b;
  /* Before c must be 4294967295 */ c=0;
  d=a&b;
  /* Before d must be 0 */ d=0;
  e=a^b;
  /* Before e must be 4294967295 */ e=0;
  f=~(a|b);
  /* Before f must be 0 */ f=0;
  g=~(a&b);
  /* Before g must be 4294967295 */ g=0;
  h=~(a^b);
  /* Before h must be 0 */ h=0;
  i=~a;
  /* Before i must be 2863311530 */ i=0;
  j=~b;
  /* Before j must be 1431655765 */ j=0;

  a=0x00000000;
  b=0x00000000;
  c=a|b;
  /* Before c must be 0 */ c=0;
  d=a&b;
  /* Before d must be 0 */ d=0;
  e=a^b;
  /* Before e must be 0 */ e=0;
  f=~(a|b);
  /* Before f must be 4294967295 */ f=0;
  g=~(a&b);
  /* Before g must be 4294967295 */ g=0;
  h=~(a^b);
  /* Before h must be 4294967295 */ h=0;
  i=~a;
  /* Before i must be 4294967295 */ i=0;
  j=~b;
  /* Before j must be 4294967295 */ j=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
