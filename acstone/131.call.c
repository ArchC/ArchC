/**
 * @file      131.call.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:21 -0300
 * @brief     It is a simple main function that uses calls functions arg int, ret int.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  int tmp;
  
  tmp=funcmenos(0);
  /* Before tmp must be -1 */ tmp=0;
  
  tmp=funcmais(0);
  /* Before tmp must be 1 */ tmp=0;
  
  tmp=funcmenos(-1);
  /* Before tmp must be -2 */ tmp=0;
  
  tmp=funcmais(-1);
  /* Before tmp must be 0 */ tmp=0;
  
  tmp=funcmenos(1);
  /* Before tmp must be 0 */ tmp=0;
  
  tmp=funcmais(1);
  /* Before tmp must be 2 */ tmp=0;
  
  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif

int funcmenos(int input) {
  return(input-1); 
} 

int funcmais(int input) {
  return(input+1);
}

