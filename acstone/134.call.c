/**********************************************************************
 It is a simple main function that uses recursive Fibonacci function.
**********************************************************************/

/* Declarations */
unsigned long long int ulifibonacci(unsigned long long int n);
signed long long int slifibonacci(signed long long int n);


/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  unsigned long long int ulin;
  signed long long int slin;

  ulin=ulifibonacci(13);
  /* Before ulin must be */ ulin=0;
  slin=slifibonacci(13);
  /* Before ulin must be */ slin=0;

  return 0; 
  /* Return 0 only */
}

unsigned long long int ulifibonacci(unsigned long long int n) {
  if(n < 3)
    return 1;
  else
    return(ulifibonacci(n-1)+
	   ulifibonacci(n-2));
}

signed long long int slifibonacci(signed long long int n) {
  if(n < 3)
    return 1;
  else
    return(slifibonacci(n-1)+
	   slifibonacci(n-2));
}


/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
