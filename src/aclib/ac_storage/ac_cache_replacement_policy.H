/* ex: set tabstop=2 expandtab: */
/**
 * @file      cache_replacement_policy.h
 * @author    Alexandro Baldassin
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.1
 *
 * @brief     Cache replacement policy abstract class.
 *
 *
 * To define a new replacement policy just create a new class with this one
 * as its super class. Implement the required 3 methods below.
 *
 */

#ifndef cache_replacement_policy_h
#define cache_replacement_policy_h



class ac_cache_replacement_policy
{
public:

  // constructor
  ac_cache_replacement_policy(unsigned int num_blocks, unsigned int assoc) : 
          m_num_blocks(num_blocks), m_assoc(assoc)
        {}

  // called when block 'index' is written to
  virtual void block_written(unsigned int block_index) =0;

  // called when block 'index' is read from
  virtual void block_read(unsigned int block_index) =0;

  // called to decide which block must be replaced
  // must return the offset of the block to be replaced (values between 0
  // and m_assoc-1) within the set (given by set_index)
  virtual unsigned int block_to_replace(unsigned int set_index) =0;


protected:

  unsigned int m_num_blocks;
  unsigned int m_assoc;

};


#endif /* cache_replacement_policy_h */
