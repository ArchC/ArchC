/**
 * @file      142.array.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   1.0
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses a kind of square matrix multiplication.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {

  signed long long int a[10][10];
  signed long long int b[10][10];
  signed long long int c[10][10];

  unsigned char i,j,k;
  
  unsigned char error=0;

  for(i=0 ; i<10 ; i++) /* A = I */
    for(j=0 ; j<10 ; j++) {
      if(i==j)
	a[i][j]=1;
      else
	a[i][j]=0;
    }

  for(i=0 ; i<10 ; i++) /* B = X */
    for(j=0 ; j<10 ; j++)
      b[i][j]=((signed long long int)i+(signed long long int)j);
 
  for(i=0 ; i<10 ; i++) /* C = 0 */
    for(j=0 ; j<10 ; j++)
      c[i][j]=0;
  
  for(i=0 ; i<10 ; i++) /* C = A . B */
    for(j=0 ; j<10 ; j++)
      for(k=0 ; k<10 ; k++)
	c[i][j]=c[i][j]+(a[i][k]*b[k][j]);

  for(i=0 ; i<10 ; i++) /* B = C ? */
    for(j=0 ; j<10 ; j++)
      if(b[i][j] != c[i][j])
	error=1;
  
  /* Before error must be 0 */ error=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
