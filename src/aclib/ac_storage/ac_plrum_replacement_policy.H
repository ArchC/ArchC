/* ex: set tabstop=2 expandtab: */
/**
 * @file      plrum_replacement_policy.h
 * @author    Alexandro Baldassin
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.1
 *
 * @brief     PLRUm replacement policy class.
 *
 *
 *  PLRUm stands for 'Pseudo Least Recently Used based on MRU bits'. For 
 *  further details please refer to the paper: "Performance Evaluation of 
 *  Cache Replacement Policies for the SPEC CPU2000 Benchmark Suite" by
 *  Al-Zoubi et al.
 *
 */

#ifndef plrum_replacement_policy_h
#define plrum_replacement_policy_h


#include <iostream>
#include <cstdlib>
#include "ac_cache_replacement_policy.H"



class ac_plrum_replacement_policy : public ac_cache_replacement_policy
{
public:

  // constructor
  ac_plrum_replacement_policy(unsigned int num_blocks, unsigned int assoc) : 
          ac_cache_replacement_policy(num_blocks, assoc)
  {

    switch (assoc) {

      case 1: return;
      case 2:
        mru_all_bits_set = 0x3;
        break;
      case 4:
        mru_all_bits_set = 0xF;
        break;
      case 8:
        mru_all_bits_set = 0xFF;
        break;
      case 16: 
        mru_all_bits_set = 0xFFFF;
        break;
      case 32:
        mru_all_bits_set = 0xFFFFFFFF;
        break;
      default:
			std::cout << "Policy does not support the specified associativity." << std::endl;
			std::exit(1);
        break;
    }

    // local storage
    mru_bits = new uint32_t[num_blocks/assoc];
    for (unsigned int i=0; i<num_blocks/assoc; i++)
      mru_bits[i] = 0;
  }

  inline void block_written(unsigned int block_index) 
  { read_written_block(block_index); }

  inline void block_read(unsigned int block_index) 
  { read_written_block(block_index); }

  // choose block to replace based on the mru_bits
  inline unsigned int block_to_replace(unsigned int set_index) 
  {
    unsigned int block_index = 0;
    unsigned int block_bits = mru_bits[set_index];

    for (unsigned int i=1; i & block_bits; i <<= 1) 
      block_index++;
   
    //cout << "Must replace a block from set " << dec << set_index << endl;
    //cout << "Replacing block " << dec << block_index << " (pLRU policy) -> ";
    //cout << "block bits " << hex << block_bits << endl << endl;
    
    return block_index;
  }

  ~ac_plrum_replacement_policy() { delete [] mru_bits; }

private:
  // each element in the array holds the MRU bits in a given set
  // bit 0 holds the MRU bit of the first block of the set, bit 1 holds the
  // MRU bit of the second, and so on.
  uint32_t *mru_bits;

  uint32_t mru_all_bits_set;

  // both read and write block use the same update mechanism
  void read_written_block(unsigned int block_index)
  {
    unsigned int block_bit = 1 << (block_index % m_assoc);

    // set the corresponding bit
    mru_bits[block_index/m_assoc] |= block_bit;

    // if all bits are set, clear them and leave only the current bit set
    if (mru_bits[block_index/m_assoc] == mru_all_bits_set) {
      mru_bits[block_index/m_assoc] = block_bit;
    }
  }

};

#endif /* plrum_replacement_policy_h */
