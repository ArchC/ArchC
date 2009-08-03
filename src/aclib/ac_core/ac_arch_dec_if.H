/**
 * @file      ac_arch_dec_if.H
 * @author    Thiago Massariolli Sigrist
 *            The ArchC Team
 *            http://www.archc.org/
 *
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br/
 *
 * @version   1.0
 * @date      Fri Nov 18 19:53:16 2005
 * 
 * @brief     Defines an ac_arch specialization that contains GetBits()
 * 
 * @attention Copyright (C) 2002-2006 --- The ArchC Team
 *
 */

//////////////////////////////////////////////////////////////////////////////

// Standard includes

// SystemC includes

// ArchC includes
#include "ac_arch.H"
#include "ac_decoder_rt.H"
#include "ac_memport.H"

//////////////////////////////////////////////////////////////////////////////

#ifndef _AC_ARCH_DEC_IF_H_
#define _AC_ARCH_DEC_IF_H_

//////////////////////////////////////////////////////////////////////////////

// Forward class declarations, needed to compile

//////////////////////////////////////////////////////////////////////////////

template <typename ac_word, typename ac_Hword> class ac_arch_dec_if:
  public ac_arch<ac_word, ac_Hword>, public ac_dec_prog_source {
public:
  explicit ac_arch_dec_if(int max_buffer) :
    ac_arch<ac_word, ac_Hword>(max_buffer) {}

  int ExpandInstrBuffer(int index) {
    //Expand the instruction buffer word by word, the number necessary to read position index
    int read = (index + 1) - this->quant;
    for(int i=0; i<read; i++){
      this->buffer[this->quant + i] = (this->IM)->read(this->decode_pc + (this->quant + i) * sizeof(ac_word));
    }
    this->quant += read;
    return this->quant;
  }

  unsigned long long GetBits(unsigned char* bu, int* quant, int last,
                             int quantity, int sign) {

    ac_word* buffer = (ac_word*) bu;

    //! Read the buffer using this macro
#define BUFFER(index) ((index<*quant) ? (buffer[index]) : (*quant=ExpandInstrBuffer(index),buffer[index]))

    int first = last - (quantity-1);

    int index_first = first/(sizeof(ac_word) * 8);
    int index_last = last/(sizeof(ac_word) * 8);

    unsigned long long value = 0;
    int i;
    if (!this->ac_mt_endian) {
      //big-endian: first  last
      //          0xAA BB CC DD

      //Read words from first to last
      for (i=index_first; i<=index_last; i++) {
        value <<= (sizeof(ac_word) * 8);
        value |= BUFFER(i);
      }

      //Remove bits before last
      value >>= (sizeof(ac_word) * 8) - (last%(sizeof(ac_word) * 8) + 1);

    }

    else {
      //little-endian: last  first
      //             0xAA BB CC DD

      //Read words from last to first
      for (i=index_last; i>=index_first; i--) {
        value <<= (sizeof(ac_word) * 8);
        value |= BUFFER(i);
      }

      //Remove bits before first
      value >>= first%(sizeof(ac_word) * 8);

    }

    //Mask to the size of the field
    value &= ((unsigned)(~0LL)) >> (64-((unsigned)quantity));

    //If signed, sign extend if necessary
    if (sign && ( value >= (unsigned)(1 << (quantity-1)) ))
      value |= (~0LL) << quantity;

    return value;
#undef BUFFER
  }

};

//////////////////////////////////////////////////////////////////////////////

#endif // _AC_ARCH_DEC_IF_H_
