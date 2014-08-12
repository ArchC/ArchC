/* ex: set tabstop=2 expandtab: */
/**
 * @file      random_replacement_policy.h
 * @author    Alexandro Baldassin
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.1
 *
 * @brief     Random replacement policy class.
 *
 *
 */

#ifndef random_replacement_policy_h
#define random_replacement_policy_h


#include "ac_cache_replacement_policy.H"



class ac_random_replacement_policy : public ac_cache_replacement_policy
{
public:

  // constructor
  ac_random_replacement_policy(unsigned int num_blocks, unsigned int associativity) : 
          ac_cache_replacement_policy(0, associativity)
        {}

  // nothing to be done
  inline void block_written(unsigned int block_index) {};

  // nothing to be done
  inline void block_read(unsigned int block_index) {};

  // random generator seed must be initialised elsewhere
  inline unsigned int block_to_replace(unsigned int set_index) 
  {
    return rand()%m_assoc;
  }

};

#endif /* random_replacement_policy_h */
