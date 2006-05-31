/* ex: set tabstop=2 expandtab: */

#include <stdio.h>
#include "sysdep.h"
#include `"opcode/'___arch_name___`.h"'

acasm_opcode opcodes[] = {
___opcode_table___
};

const acasm_symbol udsymbols[] = {
___symbol_table___
};

const char *pseudo_instrs[] = {
___pseudo_table___
};

/*
 * 1 = big, 0 = little
 *
 * limitations:
 * . endianess affects 8-bit bytes
 * . maximum word size: 64-bit
 */
long long getbits(unsigned int bitsize, char *location, int endian)
{
  long long data = 0;
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
	
  if (bits_remain) number_bytes++;

  int index, inc;
  if (endian == 1) { /* big */
    index = 0;
    inc = 1;		
  }
  else { /* little */
    index = number_bytes - 1;
    inc = -1;
  }
	
  while (number_bytes) {
    data = (data << 8) | (unsigned char)location[index];
		
    index += inc;
    number_bytes--;
  }	

/*
 * if bitsize is not multiple of 8 then clear/pack the remaining bits
 */
  long long mask = 0;
  for (; bitsize; bitsize--) 
    mask = (mask << 1) | 1;
	
  if (bits_remain) {
    if (endian == 1) {
      long long temp = data >> (8 - bits_remain);
      temp &= (mask << bits_remain);
			
      data = temp | (data & ~(mask << bits_remain));
    }
  }
	
  return data & mask;
}


void putbits(unsigned int bitsize, char *location, long long value, int endian)
{
  unsigned int number_bytes = (bitsize / 8);
  unsigned int bits_remain = bitsize % 8;
  unsigned char bytes[8];
  unsigned int i;
  int index;
  unsigned char mask = 0;

  /*
	* Fill the bytes array so as not to depend on the host endian
	*
	* bytes[0] -> least significant byte
	*/
  for (i=0; i<8; i++) 
    bytes[i] = (value & (0xFF << (i*8))) >> 8*i;

  if (bits_remain) {
    for (i=0; i<bits_remain; i++)
	   mask = (mask << 1) | 1;	
  }
	 
  if (endian == 0) { /* little */
    index = 0;

    while ((unsigned int)index != number_bytes) {    
      location[index] = bytes[index];
	  
      index++;
    }

	 if (bits_remain) 
     location[number_bytes] = (location[number_bytes] & ~mask) | (bytes[number_bytes] & mask);
	 	 
  }
  else { /* big */
    index = number_bytes - 1;
	
    i = 0;	 
    while (index >= 0) {    
      if (bits_remain) 
        location[i] = ((bytes[index+1] & mask) << (8-bits_remain)) | (bytes[index] >> bits_remain);
      else		 
        location[i] = bytes[index];

      i++;
      index--;
    }

    if (bits_remain) 
      location[number_bytes] = (bytes[0] & mask) | (location[number_bytes] & ~mask);
  }	
}

const int num_opcodes = ((sizeof opcodes) / (sizeof(opcodes[0])));
const int num_symbols = ((sizeof udsymbols) / (sizeof(udsymbols[0])));
const int num_pseudo_instrs = ((sizeof pseudo_instrs) / (sizeof(pseudo_instrs[0])));
