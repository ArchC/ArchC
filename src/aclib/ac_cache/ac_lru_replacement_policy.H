/* ex: set tabstop=2 expandtab: */
/**
 * @file      lru_replacement_policy.h
 * @author    Henrique Dante de Almeida
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.1
 *
 * @brief     LRU (least recently used) replacement policy class.
 *
 *
 */

#ifndef lru_replacement_policy_h
#define lru_replacement_policy_h


#include "ac_cache_replacement_policy.H"
#include <stdint.h>


class ac_lru_replacement_policy : public ac_cache_replacement_policy
{
public:

  // constructor
  ac_lru_replacement_policy(unsigned int num_blocks, unsigned int assoc) : 
          ac_cache_replacement_policy(num_blocks, assoc), sequence(0), count(0)
  {
	if (assoc > 1) {
	    	unsigned index_size = num_blocks/assoc;
	    	count = index_size;
	    	sequence = new uint8_t*[count];
	    	for (unsigned i = 0; i < count; i++) {
	    		sequence[i] = new uint8_t[assoc];
	    		// Sane default values
	    		for (unsigned j = 0; j < assoc; j++) {
	    			sequence[i][j] = assoc-j-1;
			}
		}
	}
  }

  inline void block_written(unsigned int block_index) 
  {
  	if (m_assoc <= 1) return;
  	unsigned a = block_index % m_assoc;
  	unsigned i = block_index / m_assoc;
  	
	unsigned x = 0;
	while(sequence[i][x] != a) {
		x++;
	};
	while (x > 0) {
		sequence[i][x] = sequence[i][x-1];
		x--;
	};
	sequence[i][0] = a; 	
  };

  inline void block_read(unsigned int block_index)
  {
  	block_written(block_index);
  };

  // returns current counter and updates it
  inline unsigned int block_to_replace(unsigned int set_index) 
  {
	if (m_assoc <= 1) return 0;
	return sequence[set_index][m_assoc-1]; 
  }

  ~ac_lru_replacement_policy()
  {
  	if (count) {
  		for (size_t i = 0; i < count; i++) {
			delete[] sequence[i];
		}
		delete[] sequence;
	}
  }
private:
  uint8_t **sequence;
  size_t count;
};

#endif /* lru_replacement_policy_h */
