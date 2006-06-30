/**
 * @file      126.loop.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses implemented simple fatorial loop.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned long long int nu;
  unsigned long long int resultu;
  unsigned long long int iu;

  signed long long int ns;
  signed long long int results;
  signed long long int is;

  nu=25;
  resultu=1;
  iu=nu;
  while(iu>1) {
    resultu=resultu*iu;
    iu--;
  }
  /* Before must be 7034535277573963776 */ resultu=0;

  ns=25;
  results=1;
  is=ns;
  while(is>1) {
    results=results*is;
    is--;
  }
  /* Before must be 7034535277573963776 */ results=0;
  

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
