/*****************************************************************************
                             PowerSC Library

                           Copyright 2004-2006
                       Computer Systems Laboratory
                          All Rights Reserved

 PERMISSION IS GRANTED TO USE, COPY AND REDISTRIBUTE THIS SOFTWARE FOR
 NONCOMMERCIAL EDUCATION AND RESEARCH PURPOSES, SO LONG AS NO FEE IS CHARGED,
 AND SO LONG AS THE COPYRIGHT NOTICE ABOVE, THIS GRANT OF PERMISSION, AND THE
 DISCLAIMER BELOW APPEAR IN ALL COPIES MADE; AND SO LONG AS THE NAME OF THE
 COMPUTER SYSTEMS LABORATORY IS NOT USED IN ANY ADVERTISING OR PUBLICITY
 PERTAINING TO THE USE OR DISTRIBUTION OF THIS SOFTWARE WITHOUT SPECIFIC,
 WRITTEN PRIOR AUTHORIZATION. PERMISSION TO MODIFY OR OTHERWISE CREATE
 DERIVATIVE WORKS OF THIS SOFTWARE IS NOT GRANTED.

 THIS SOFTWARE IS PROVIDED AS IS, WITHOUT REPRESENTATION AS TO ITS FITNESS
 FOR ANY PURPOSE, AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR
 IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE STATE UNIVERSITY
 OF CAMPINAS, THE INSTITUTE OF COMPUTING AND THE COMPUTER SYSTEMS LABORATORY
 SHALL NOT BE LIABLE FOR ANY DAMAGES, INCLUDING SPECIAL, INDIRECT, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, WITH RESPECT TO ANY CLAIM ARISING OUT OF OR IN
 CONNECTION WITH THE USE OF THE SOFTWARE, EVEN IF IT HAS BEEN OR IS HEREAFTER
 ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 *****************************************************************************/

/*****************************************************************************

  psc_randgen.cpp -- Pseudo-random number generator utility functions

  Original Author: Felipe Vieira Klein <felipe.klein@ic.unicamp.br>, IC/UNICAMP, 2004.
  
  LSC (Computer Systems Laboratory) home page: http://www.lsc.ic.unicamp.br

 *****************************************************************************/
#include <math.h>
#include "psc_randgen.h"

namespace psc_util 
{

const int PSC_RAND_MAX = 2147483647;

static int jseed, ifrst;

void psc_util_srand( int iseed )
{
   jseed = iseed;
   ifrst = 0;
}


float psc_util_rand()
{
   const int mplier = 16807;
   const int modlus = PSC_RAND_MAX;
   const int mobymp = 127773;
   const int momdmp = 2836;

   float ret;
   int hvlue, lvlue, testv;

   static int nextn;

   if ( ifrst == 0 ) {
      nextn = jseed;
      ifrst = 1;
   }

   hvlue = nextn / mobymp;
   lvlue = nextn % mobymp;
   testv = (mplier * lvlue) - (momdmp * hvlue);

   if ( testv > 0 ) {
      nextn = testv;
   } else {
      nextn = testv + modlus;
   }

   ret = (float)nextn / (float)modlus;

   return( ret );
}


// return a number between 0 and max
int psc_util_rand_num( int max )
{
   float tmp = psc_util_rand() * (float)max;

   return ((int)tmp) + 1;
}

int psc_util_rand_num( int max, unsigned char bits)
{
   int tmp;
   unsigned int mask = (unsigned int)pow( 2.0, bits );
   mask = mask - 1;
   
   tmp = max & mask;

   return( psc_util_rand_num( tmp ) );
}

} // namespace psc_util

