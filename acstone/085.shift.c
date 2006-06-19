/**
 * @file      085.shift.c
 * @author    The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   version?
 * @date      Mon, 19 Jun 2006 15:33:22 -0300
 * @brief     It is a simple main function that uses shifts and some logic operators.
 *
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

/* The file begin.h is included if compiler flag -DBEGINCODE is used */
#ifdef BEGINCODE
#include "begin.h"
#endif

int main() {
  
  unsigned long long int abcdefgh;
  unsigned long long int hgfedcba;
  unsigned int abcd;
  unsigned int dcba;
  unsigned short int ab;
  unsigned short int ba;

  abcdefgh=0x8899AABBCCDDEEFFULL;
  hgfedcba=(((abcdefgh & 0xFF00000000000000ULL)>>56) |
	    ((abcdefgh & 0x00FF000000000000ULL)>>40) |
	    ((abcdefgh & 0x0000FF0000000000ULL)>>24) |
	    ((abcdefgh & 0x000000FF00000000ULL)>>8) |
	    ((abcdefgh & 0x00000000FF000000ULL)<<8) |
	    ((abcdefgh & 0x0000000000FF0000ULL)<<24) |
	    ((abcdefgh & 0x000000000000FF00ULL)<<40) |
	    ((abcdefgh & 0x00000000000000FFULL)<<56));
  /* Before hgfedcba must be 18441921395520346504 */ hgfedcba=0;

  abcd=0xAABBCCDD;
  dcba=(((abcd & 0x000000FF)<<24) | ((abcd & 0x0000FF00)<<8) |
	((abcd & 0x00FF0000)>>8) | ((abcd & 0xFF000000)>>24));
  /* Before dcba must be 3721182122 */ dcba=0;

  ab=0xAABB;
  ba=(((ab & 0x00FF)<<8) | ((ab & 0xFF00)>>8));
  /* Before ba must be 48042 */ ba=0;

  return 0; 
  /* Return 0 only */
}

/* The file end.h is included if compiler flag -DENDCODE is used */
#ifdef ENDCODE
#include "end.h"
#endif
