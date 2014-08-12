/* ex: set tabstop=2 expandtab: */
/**
 * @file      fifo_replacement_policy.h
 * @author    Alexandro Baldassin
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.1
 *
 * @brief     FIFO replacement policy class.
 *
 * Also known as Round Robin policy. It replaces a cache block in a sequential
 * order. Each set maintains a circular counter pointing to the next block 
 * to be replaced. On a miss the counter is updated.
 *
 *
 */

#ifndef fifo_replacement_policy_h
#define fifo_replacement_policy_h


#include "ac_cache_replacement_policy.H"
#include <stdint.h>


class ac_fifo_replacement_policy : public ac_cache_replacement_policy
{
public:

  // constructor
  ac_fifo_replacement_policy(unsigned int num_blocks, unsigned int assoc) : 
          ac_cache_replacement_policy(num_blocks, assoc),counter(NULL)
  {

    // local storage for the set counters
	  printf("\nassoc=%d, m_assoc=%d", assoc, m_assoc);
    if (m_assoc != 1) {
    	counter = new uint8_t[num_blocks/assoc];
    	for (int i=0; i<num_blocks/assoc; i++) counter[i]=0;
    }
    else counter = NULL;
  }

  // nothing to be done
  inline void block_written(unsigned int block_index) {};

  // nothing to be done
  inline void block_read(unsigned int block_index) {};

  // returns current counter and updates it
  inline unsigned int block_to_replace(unsigned int set_index) 
  {
    uint8_t next_one = counter[set_index];
    counter[set_index] = (next_one+1) % m_assoc;

    //cout << "Replacing block (fifo policy)..." << endl;
    //cout << "Set " << set_index << " -> line " << (int)next_one << " chosen" << endl;
    
    return (unsigned int)next_one;
  }

  virtual ~ac_fifo_replacement_policy() { if (this->m_assoc != 1) delete [] counter; }

private:
  // each set has a counter
  uint8_t *counter;

};

#endif /* fifo_replacement_policy_h */
