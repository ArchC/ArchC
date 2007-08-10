

/*
 Shared functions used by ArchC-generated modules (bfd, opcodes)
*/

#include <assert.h>
#include `"share-'___arch_name___`.h"'

const mod_fnptr modfn[] = {
___modenc_pointers___
};

const mod_fnptr modfndec[] = {
___moddec_pointers___
};

const unsigned int num_modfn = ((sizeof modfn) / (sizeof(modfn[0])));

const unsigned int num_modfndec = ((sizeof modfndec) / (sizeof(modfndec[0])));


const acasm_operand operands[] = {
___operand_table___
};

const unsigned int num_oper_id = ((sizeof operands) / (sizeof(operands[0])));


void print_operand_info(FILE *stream, unsigned int indent, unsigned int opid)
{
  fprintf(stream, "%*s<\"%s\", ", indent, "", operands[opid].name);
  
  switch (operands[opid].type) {
    case op_exp: fprintf(stream, "exp, ");
      break; 
    case op_imm: fprintf(stream, "imm, ");
      break;
    case op_addr: fprintf(stream, "addr, ");
      break;
    case op_userdef: fprintf(stream, "userdef, ");
      break;
  }

  printf(stream, "mod_type = %u ", operands[opid].mod_type);
/*
  switch (operands[opid].mod_type) {
    case mod_default: fprintf(stream, "mod_default, ");
      break;
    case mod_low: fprintf(stream, "mod_low, ");
      break;
    case mod_high: fprintf(stream, "mod_high, ");
      break;
    case mod_aligned: fprintf(stream, "mod_aligned, ");
      break;
    case mod_pcrel: fprintf(stream, "mod_pcrel, ");
      break;
    case mod_carry: fprintf(stream, "mod_carry, ");
      break;
  }
*/
  fprintf(stream, "0x%08X:", operands[opid].fields);

  unsigned int numf = get_num_fields(operands[opid].fields);
  fprintf(stream, "%d:[ ", numf);

  unsigned int count;
  for (count=0; count < numf; count++) {
    fprintf(stream, "%d ", get_field_id(operands[opid].fields, count));
  }
  fprintf(stream, "], ");

  fprintf(stream, "%d>\n", operands[opid].reloc_id);
}


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


unsigned int get_num_fields(unsigned int encoded_field)
{
  /* count the number of 1's in the bit field */
  unsigned numf = 0;
  unsigned int shift = 1;

  for (; shift; shift <<= 1) {
    if (encoded_field & shift)
      numf++;
  }

  return numf;
}

unsigned int get_field_id(unsigned int encoded_field, unsigned int pos)
{
  unsigned int id = 999;
  unsigned int counter = 0;
  unsigned int shift = 1;

  for (; shift; shift <<= 1) {
    if (encoded_field & shift) {
      if (pos == 0) return counter;
      pos--;
    }
    counter++;
  }

  return id;
}


void encode_cons_field(unsigned int *image, mod_parms *mp, unsigned int oper_id)
{
  unsigned int lt;
  unsigned int fid; 
  int i;
  unsigned int nf = get_num_fields(operands[oper_id].fields);

  assert(operands[oper_id].mod_type <= num_modfn);

  mp->addend = operands[oper_id].mod_addend;
  mp->error = 0;

  if (operands[oper_id].mod_type == mod_default) {

    for (i=nf-1; i >= 0; i--) {
      fid = get_field_id(operands[oper_id].fields, i);
      mp->field[fid] = mp->input;
      mp->input >>= get_field_size(operands[oper_id].format_id, fid); 
    }
  } 
  else {
    for (i=0; i < ___max_fields___ ; i++)
      mp->field[i] = 0;
    mp->output = 0;    
    (*modfn[operands[oper_id].mod_type])(mp);
    if (mp->output !=0)
      mp->field[get_field_id(operands[oper_id].fields,0)] = mp->output;
  }

  for (lt=0; lt < nf; lt++) {
    fid = get_field_id(operands[oper_id].fields, lt);

    ac_encode_insn(operands[oper_id].format_id, fid, mp->field[fid], image);
  }

}

unsigned long get_insn_size(unsigned long insn_fmt)
{
___insnsize_function___
}


unsigned long ac_encode_insn(unsigned long insn_type, int field_id, unsigned long value, unsigned int *image)
{
___encoding_function___
}


unsigned int ac_get_field_value(unsigned int insn_type, int field_id, unsigned int value)
{
___fieldvalue_function___
}


unsigned long get_field_size(unsigned long insn_fmt, int field_id)
{
___fieldsize_function___
}


#define ac_modifier_encode(modifier) void modifier_ ##modifier ## _encode(mod_parms *reloc)
#define ac_modifier_decode(modifier) void modifier_ ##modifier ## _decode(mod_parms *reloc)


/* dummy modifiers */
ac_modifier_encode(default) {}
ac_modifier_decode(default) {}

___modifiers___

