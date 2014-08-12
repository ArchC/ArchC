/* ex: set tabstop=2 expandtab: */
/**
 * @file      cache_bhv.h
 * @author    Alexandro Baldassin
 *            
 *            Computer Systems Laboratory (LSC)
 *            IC-UNICAMP
 *            http://www.lsc.ic.unicamp.br
 *
 * @version   0.4
 *
 * @brief     Cache behaviour base class.
 *
 *
 * 0.4 - 
 *       . replacement algorithm now is implemented in a separate class and
 *       must be choosen while instatiating the cache (defaults to random);
 *       . added support for counting the number of block evictions.
 *
 * 0.3 - replacement algorithm must be specified in a derived class.
 *
 *
 *
 * The present cache class implements basic cache behaviour 
 * (read/write blocks). I tried to make the implementation as generic as I 
 * could. 
 *
 * It should be noted that the present cache class does not implement 
 * CPU -> cache 'read' and 'write' operations. These operations should be 
 * implemented by a derived class, making use of the very basic operations 
 * on cache blocks provided here.
 *
 *
 * *** How to ... ***
 *
 * 1. Instantiate
 *
 *  Two simple data types should be specified as template parameters:
 *
 *   ADDRESS: range of addressable memory positions (i.e., 'unsigned int' 
 *            means that 32-bit addresses are used - 2^32 memory positions
 *            available).
 *
 *   DATA:    minimum addressable data (i.e., 'unsigned int' means that
 *            each cache block has a 4-byte value).
 *
 *  Note that each address contains an entire data element of size DATA. For
 *  instance, if DATA == unsigned int, then address 0 contains an entire
 *  4-byte value, address 1 contains the next 4-byte value and so on. In this
 *  case, special care should be taken if the address is given in terms of 
 *  bytes -> you will need to divide the address by 4 before indexing the 
 *  memory, and then select one of the four bytes accordingly. 
 *
 *  Moreover, you should create a 'cache_status_t' ADT with cache status
 *  information. This ADT is also specified as a template when instatiating 
 *  the cache object. Required operations which should be provided by the ADT:
 *
 *  bool is_invalid() -> true if cache line is invalid
 *  void print(ostream &) -> write a string with the status into the stream
 *
 *
 *  Four parameters can be specified in the constructor. The first three of them
 *  (below) must be power of 2:
 *   
 *    number of blocks: guess what ;)  
 *    block size      : size (in units of DATA) of each block 
 *    associativity   : number of blocks in a set (cannot be larger than the
 *                      number of blocks). If associativity == number of
 *                      blocks, the cache is said to be fully associative.
 *                      If associativity == 1, the cache is said to be
 *                      direct mapped (default).
 *    rep. policy     : cache replacement policy
 *
 *
 * 2. Implement CPU -> cache read/write operations in derived classes 
 *
 *  a) The first method which *must* be called is 'get_block()'. If cache 
 *  statistics are desired call either 'get_block_for_read()' or
 *  'get_block_for_write()' for reads and writes, respectively.
 *
 *  b) Check whether get_block() returned true or false.
 *
 *   b1) true (cache hit)   -> you can safely call:
 *                       read_block_single()  -> read single data
 *                       write_block_single() -> write single data
 *                       read_block()    -> read entire block
 *                       write_block()   -> write entire block
 *       check the methods description for further details.
 *
 *   b2) false (cache miss) -> call get_available_block(). This method will
 *       accordingly choose the most suited block to be used/replaced
 *
 *       b2.1) you can use block_status() to check whether the chosen block
 *       can be promptly used (INVALID) or not. If the block is invalid then
 *       you can replace it with data from main memory and read/write from/to
 *       it afterwards. Otherwise you might need to write the block back to
 *       memory before replacing it. Use block_address() to get the physical
 *       address of the data stored in the current block.
 *
 *
 * Useful for debugging purposes (if defined)
 *
 *  CACHE_BHV_MSG   -> useful messages
 *
 *
 * @attention Copyright (C) 2007 - LSC, UNICAMP.
 */


#ifndef cache_bhv_h
#define cache_bhv_h


#include <iostream>
#include <cstdlib>     

#include "ac_cache_replacement_policy.H" 
#include "ac_random_replacement_policy.H" 
#include "ac_fifo_replacement_policy.H" 
#include "ac_plrum_replacement_policy.H" 


using namespace std;

namespace whitebox { class cache_bhv_test; }

//enum replacement_policy_t {random_policy=0, fifo_policy=1, plrum_policy=2};



/*
 * Forward declarations.
 */
template <
	unsigned index_size,
	unsigned block_size,
	unsigned associativity,
	typename cpu_word,
	typename ADDRESS,
	typename cache_status_t,
	typename replacement_policy
> 
class cache_bhv;

template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
ostream &operator << (ostream &fsout, 
					  const cache_bhv<index_size, block_size, associativity, cpu_word,
					  ADDRESS, cache_status_t, replacement_policy> &cache); 


/**
 * Cache class
 *
 */
template <
	unsigned index_size,
	unsigned block_size,
	unsigned associativity,
	typename cpu_word,
	typename ADDRESS,
	typename cache_status_t,
	typename replacement_policy
> 
class cache_bhv 
{
public:

  // friend << operator for printing objects of this class
  friend ostream &operator << <> (ostream &, const cache_bhv &);
  friend class whitebox::cache_bhv_test;

  /** Constructor.
   *
   * @param nb Number of blocks.
   * @param bs Block size (in units of DATA).
   * @param assoc Number of blocks per set (default = 1). If nb == assoc,
   *              the cache is fully associative. If assoc == 1, the cache
   *              is direct mapped.
   * @param rp Cache replacement policy (defaults to random).
   *
   * Note: First 3 parameters should be a power of 2.
   */
/*  cache_bhv(unsigned int nb, unsigned int bs, unsigned int assoc = 1, 
            replacement_policy_t rp = random_policy); */
	cache_bhv();

  virtual void print_statistic(ostream &fsout) const;

  // returns read statistics
  virtual inline unsigned long long int number_read_hit(void) const 
  { return m_read_hit; } 
  virtual inline unsigned long long int number_read_miss(void) const
  { return m_read_miss; } 

  // returns write statistics
  virtual inline unsigned long long int number_write_hit(void) const 
  { return m_write_hit; } 
  virtual inline unsigned long long int number_write_miss(void) const
  { return m_write_miss; } 

  virtual inline unsigned long long int number_block_eviction(void) const
  { return m_evictions; }


// destructor
  ~cache_bhv() {
  }

protected:

  // replacement policy object pointer
  //cache_replacement_policy *m_rep_pol;

	replacement_policy m_rep_pol;
	
  /** Cache block structure. 
   *
   * Each block is made up of a status field, a tag and the data.
   * The status field is a template parameter.
   */
  struct cache_block_t {
    cache_status_t *status;
    ADDRESS        *tag;
    cpu_word       *data;
    unsigned int   index; /* not really needed, but helps in retrieving the 
                             address assigned to a random cache block */
						  // Note: this index is multiplied by associativity
  };


  /**
   * Get cache block.
   *
   * This *must* be the first method to be called and will properly
   * initialise the access to the cache blocks. 
   * 
   * @param addr Address of the data. Note that addresses are DATA-based,
   *             not necessarily byte-based.
   *
   * @return true if cache hit -> ok to perform read/write block operations.  
   *         false if cache miss -> call get_available_block() to get an
   *         available block.
   */
  inline bool get_block(ADDRESS addr)
  { return _internal_get_block(addr, m_current_sa, m_current_block); }

  // implicitly returns a cache_block_t reference if cache contains data
  inline bool get_block(ADDRESS addr, cache_block_t &cb)
  {
    split_address_t sa;
    return _internal_get_block(addr, sa, cb);
  }


public:
  /**
   * Get available block.
   *
   * It will properly find an available block. After calling this method 
   * you can use the remaining block operations to find out the state of
   * the block (if it should be evicted) and also issue read/write operations.
   */
  void get_available_block(void);

  /**
   * Write cache block (single version).
   * 
   * Writes a single DATA element into the cache. The location where the data
   * will be written should have been already select by earlier calls to 
   * get_block() or get_available_block().
   *
   * @param st     New status of the block to be written.
   * @param value  Value to be written.
   *
   */
  void write_block_single(const cache_status_t &st, const cpu_word *value, unsigned length);

  // same as above, but the block status is not changed.
  void write_block_single(const cpu_word *value, unsigned length);

 /**
   * Write entire cache block.
   * 
   * Writes a block of DATA elements into the cache. The size of the block
   * is an initialisation parameter (nb).
   *
   * @param st        New status of the block to be written.
   * @param blockdata Pointer to the DATA array.
   *
   */
  void write_block(const cache_status_t &st, const cpu_word *blockdata);

  // same as above, but the block status is not changed.
  void write_block(const cpu_word *blockdata);
 
  /**
   * Read cache block (single version).
   * 
   * Reads a single DATA element from the cache. The location where the data
   * will be read should have been already selected by earlier calls to 
   * get_block() or get_available_block().
   *
   * @return A DATA element.
   *
   */
  const cpu_word *read_block_single(void);

  /**
   * Read entire cache block.
   * 
   * Reads a block of DATA elements from the cache.
   *
   * @return Pointer to the DATA array.
   *
   */
  const cpu_word *read_block(void);


   /*
   * Block fields inline methods (set/get).
   */

  // returns the tag of the current cache block
  inline ADDRESS block_tag(void) const
  { return *(m_current_block.tag); }

  // returns the tag of 'cb' cache block
  inline ADDRESS block_tag(const cache_block_t &cb) const
  { return *(cb.tag); }

  // returns the status of the current cache block
  inline cache_status_t &block_status(void) const
  { return *(m_current_block.status); }

  // returns the status of 'cb' cache block
  inline cache_status_t &block_status(const cache_block_t &cb) const
  { return *(cb.status); }

  // returns the data pointer of a 'cb' cache block
  inline const char *block_data(const cache_block_t &cb) const
  { return cb.data; }

  // set the status of a cache block 'cb' to 'cs'
  inline void set_block_status(cache_block_t &cb, const cache_status_t &cs)
  { *(cb.status) = cs; }

  inline void set_block_status(const cache_status_t &cs)
  { *(m_current_block.status) = cs; }

  inline unsigned int block_index(void) const
  { return m_current_block.index; }

  inline unsigned int block_index(const cache_block_t &cb) const
  { return cb.index; }

  // returns the block address (phys. addr) of the current cache block
  inline ADDRESS block_address(void) const
  {
    return ( (*(m_current_block.tag) << (m_index_bits + m_offset_bits)) |
             ((m_current_sa.index/associativity << m_offset_bits)) );
  }

  // returns the block address (phys. addr) of block cb
  inline ADDRESS block_address(const cache_block_t &cb) const
  {
    return ( (*(cb.tag) << (m_index_bits + m_offset_bits)) |
             ((cb.index/associativity << m_offset_bits)) );
  }

  // returns the pointer to the cache blocks
  // USE IT WITH EXTREME CAUTION!
  inline const cache_block_t *block_pointer(void) const
  { return m_blocks; }

  // returns the current cache block information
  inline const cache_block_t current_block(void) const
  { return m_current_block; }

  // same as get_block(), but counts read miss/hit
  inline bool get_block_for_read(ADDRESS addr)
  {
    if (get_block(addr))
      m_read_hit++;
    else {
      m_read_miss++;
      return false;
    }
    return true;
  }

  // same as get_block(), but counts write miss/hit
  inline bool get_block_for_write(ADDRESS addr)
  {
    if (get_block(addr))
      m_write_hit++;
    else {
      m_write_miss++;
      return false;
    }
    return true;
  }



private:

  /**
   * Split address structure.
   *
   * The address passed to get_block() is split into fields tag, index 
   * and offset for post processing.
   */
  struct split_address_t {
    ADDRESS tag;
    ADDRESS index; // NOTE; this index is multiplied by associativity
    ADDRESS offset;
  };

  /**
   * Split address method (private).
   *
   * @param address Address to be split (DATA-based).
   * @param sa      Reference to a split_address_t variable which
   *                will hold the split address.
   */
  inline void split_address(ADDRESS address, split_address_t &sa)
  {
    sa.tag = address >> (m_offset_bits+m_index_bits);
    sa.index = (address >> m_offset_bits) & m_index_mask;
    sa.index *= associativity;
    sa.offset = address & m_offset_mask;
  }

  /**
   * Core version of get_block().
   *
   * Parameters split_address_t and cache_block_t are passed explicitly.
   *
   */
  bool _internal_get_block(ADDRESS addr, split_address_t &sa, 
                           cache_block_t &cb);


  /*
   * Variables
   */

//  cache_block_t    *m_blocks;  /**< pointer to the cache blocks. */

  // these are set by get_block() or get_available_block()
  cache_block_t    m_current_block;  /**< Current block. */ 
  split_address_t  m_current_sa;     /**< Current split address. */

  // pointers to each cache 
  cpu_word m_cache_data[block_size*associativity*index_size/sizeof(cpu_word)];  /**< A single pointer to the whole data. */
  ADDRESS m_cache_tag[associativity*index_size];           /**< Pointer to the whole tag data. */
  cache_status_t m_cache_status[associativity*index_size]; /**< Pointer to the whole status data. */
  cache_block_t m_blocks[associativity*index_size];        /**< Pointer that organizes all the pointers above. */

  // size of the address fields (in bits)
  unsigned int m_tag_size;
  unsigned int m_index_bits;
  unsigned int m_offset_bits;

  // corresponding masks
  unsigned int m_offset_mask;
  unsigned int m_index_mask;
  unsigned int m_tag_mask;


  // statistics
  unsigned long long int m_read_miss;
  unsigned long long int m_read_hit;
  unsigned long long int m_write_miss;
  unsigned long long int m_write_hit;
  unsigned long long int m_evictions;
};



/*
 *
 * Methods implementation.
 *
 *
 */


template <
  unsigned index_size,
  unsigned block_size,
  unsigned associativity,
  typename cpu_word,
  typename ADDRESS,
  typename cache_status_t,
  typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
print_statistic(ostream &fsout) const
{
  unsigned long long int total_read = number_read_miss()+number_read_hit();
  unsigned long long int total_write = number_write_miss()+number_write_hit();

  if (total_read == 0) total_read = 1;
  if (total_write == 0) total_write = 1;

  fsout << "Cache statistics:" << endl;

  fsout << "Read:   miss: " << number_read_miss() << " (" \
          << ((number_read_miss()/(float)total_read))*100 << "%) hit: " \
          << number_read_hit() << " (" \
          << (number_read_hit()/(float)total_read)*100 << "%)" << endl;


  fsout << "Write:  miss: " << number_write_miss() << " (" \
          << ((number_write_miss()/(float)total_write))*100 << "%) hit: " \
          << number_write_hit() << " (" \
          << (number_write_hit()/(float)total_write)*100 << "%)" << endl;

  fsout << "Number of block evictions: " << number_block_eviction() << endl;
}



// constructor
template <
  unsigned index_size,
  unsigned block_size,
  unsigned associativity,
  typename cpu_word,
  typename ADDRESS,
  typename cache_status_t,
  typename replacement_policy
> 
cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS, 
          cache_status_t, replacement_policy>::
cache_bhv() :
	 m_rep_pol(index_size*associativity, associativity),
     m_read_miss(0),
     m_read_hit(0),
     m_write_miss(0),
     m_write_hit(0),
     m_evictions(0)
{	
	
	unsigned block_count = index_size*associativity;
//  m_blocks       = new cache_block_t[block_count];  // allocate memory for the blocks

  // make each data pointer from each cache block point to the right 
  // memory address
  for (unsigned int i=0; i<block_count; i++) {
    m_blocks[i].data = (m_cache_data+(block_size/sizeof(cpu_word)*i));
    m_blocks[i].tag = (m_cache_tag+i);
    *(m_blocks[i].tag) = 0;
    m_blocks[i].status = (m_cache_status+i);
    m_blocks[i].index = i;
  }


  // emmit a warning in case parameters 1 and 2 are not a power of 2
  if ((block_count & (block_count-1)) != 0) 
    cout << "Warning: number of blocks must be a power of 2." << endl;
  if ((block_size & (block_size-1)) != 0) 
    cout << "Warning: block size must be a power of 2." << endl;
  if ((block_count % associativity) != 0)
    cout << "Warning: associativity invalid." << endl;
  

  /*
   * Compute 'offset' size and mask -> log2(bs/ws)
   */
  unsigned int temp = block_size/sizeof(cpu_word);
  m_offset_bits = 0;
  m_offset_mask = 0;
  int bit_pos = 1;
  for (unsigned int i=0; i<(sizeof(ADDRESS)*8); i++) {
    if (temp & (bit_pos<<i)) break;
    m_offset_mask = (m_offset_mask << 1) | 1;
    m_offset_bits++;
  } 

  /*
   * Compute 'index' size and mask -> log2 (nb/assoc)
   */
  temp = index_size;
  m_index_bits = 0;
  m_index_mask = 0;
  bit_pos = 1;
  for (unsigned int i=0; i<(sizeof(ADDRESS)*8); i++) {
    if (temp & (bit_pos<<i)) break;
    m_index_mask = (m_index_mask << 1) | 1;
    m_index_bits++;
  } 

  /*
   * 'tag' size and mask
   */
  m_tag_size = (sizeof(ADDRESS)*8) - m_offset_bits - m_index_bits; 
  // not really needed, we just need to >> (m_offset_bits+m_index_bits)
  m_tag_mask = 0xFFFFFFFF; 

#ifdef CACHE_BHV_MSG
  cout << "tag " << m_tag_size << " index " << m_index_bits << "(" << hex << m_index_mask \
          << ") " << " offset " << m_offset_bits << "(" << hex << m_offset_mask << ")" \
          << dec << endl;
#endif

}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
bool cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
_internal_get_block(ADDRESS addr, split_address_t &sa, cache_block_t &cb)
{
  split_address(addr, sa);

  for (unsigned int i=0; i<associativity; i++) {
    if ( (! m_blocks[sa.index+i].status->is_invalid()) &&
         (block_tag(m_blocks[sa.index+i]) == sa.tag) ) {
      cb = m_blocks[sa.index+i];
      return true;
    }
  }
  return false;
}



template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
get_available_block(void)
{

  if (associativity > 1) {
  /*
   * Replacement strategy for set-associative caches
   *
   */
  // first try to find an INVALID line
  
    for (unsigned int i=0; i<associativity; i++) {
      if ( m_blocks[m_current_sa.index+i].status->is_invalid() ) {
        m_current_block = m_blocks[m_current_sa.index+i];
        return;
      }
    }

  // no INVALID block was found... choose line to evict
    unsigned int block_index = 
    m_rep_pol.block_to_replace(m_current_sa.index/associativity);

    m_current_block = m_blocks[m_current_sa.index+block_index];

  }
  else { // direct-mapped caches
    m_current_block = m_blocks[m_current_sa.index];
  }

  m_evictions++;
}



template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
write_block_single(const cache_status_t &st, const cpu_word *value, unsigned length)
{
#ifdef CACHE_BHV_MSG 
  cout << "Writing " << value << " at block index " << m_current_sa.index \
       << " (" << m_current_sa.tag << ", " << m_current_sa.offset << ")" << endl;
#endif

  unsigned max_length = block_size - m_current_sa.offset*sizeof(cpu_word);
  if (length > max_length) abort();

  m_rep_pol.block_written(m_current_block.index);
          
  *(m_current_block.status) = st;
  *(m_current_block.tag) = m_current_sa.tag;
  // tmp is required for gcc loop vectorization
  cpu_word *tmp = m_current_block.data + m_current_sa.offset;
  for (unsigned i = 0; i < length/sizeof(cpu_word); i++) {
    tmp[i] = value[i];
  }
}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
write_block_single(const cpu_word *value, unsigned length)
{
#ifdef CACHE_BHV_MSG 
  cout << "Writing " << value << " at block index " << m_current_sa.index \
       << " (" << m_current_sa.tag << ", " << m_current_sa.offset << ")" << endl;
#endif

  unsigned max_length = block_size - m_current_sa.offset*sizeof(cpu_word);
  if (length > max_length) abort();

  m_rep_pol.block_written(m_current_block.index);

  *(m_current_block.tag) = m_current_sa.tag;
  // tmp is required for gcc loop vectorization
  cpu_word *tmp = m_current_block.data + m_current_sa.offset;
  for (unsigned i = 0; i < length/sizeof(cpu_word); i++) {
    tmp[i] = value[i];
  }
}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
write_block(const cache_status_t &st, const cpu_word *blockdata)
{
#ifdef CACHE_BHV_MSG 
  cout << "Writing a block at index " << m_current_sa.index << " (" \
          << m_current_sa.tag << ")" << endl;
#endif

  *(m_current_block.status) = st;
  *(m_current_block.tag) = m_current_sa.tag;

  char *tmp = m_current_block.data; // tmp is required for loop vectorization in gcc
  for (unsigned int i=0; i<block_size/sizeof(cpu_word); i++) {
    tmp[i] = blockdata[i];
  }
}

template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
void cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
               cache_status_t, replacement_policy>::
write_block(const cpu_word *blockdata)
{
#ifdef CACHE_BHV_MSG 
  cout << "Writing a block at index " << m_current_sa.index << " (" \
          << m_current_sa.tag << ")" << endl;
#endif


  *(m_current_block.tag) = m_current_sa.tag;

  cpu_word *tmp = m_current_block.data; // tmp is required for loop vectorization in gcc

  for (unsigned int i=0; i<block_size/sizeof(cpu_word); i++) {
    tmp[i] = blockdata[i];
    //printf("\nwrite_block i=%d  %x", i, tmp[i]);
  }
}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
const cpu_word *cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
                      cache_status_t, replacement_policy>::
                      read_block_single(void)
{
#ifdef CACHE_BHV_MSG 
  cout << "Reading " << m_current_block.data[m_current_sa.offset] \
       << " from block index " << m_current_sa.index << ", offset " \
       << m_current_sa.offset << " (" << *(m_current_block.tag) \
       << ")" << endl;
#endif

  m_rep_pol.block_read(m_current_block.index);

  return &m_current_block.data[m_current_sa.offset];
}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
const cpu_word *cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS,
                      cache_status_t, replacement_policy>::
read_block(void)
{
#ifdef CACHE_BHV_MSG 
  cout << "Reading block index " << m_current_sa.index << " (" \
          << *(m_current_block.tag) << ")" << endl;
#endif

  return m_current_block.data;
}


template <
unsigned index_size,
unsigned block_size,
unsigned associativity,
typename cpu_word,
typename ADDRESS,
typename cache_status_t,
typename replacement_policy
> 
ostream &operator << (ostream &fsout, 
					  const cache_bhv<index_size, block_size, associativity, cpu_word, ADDRESS, 
					                  cache_status_t, replacement_policy> &cache)
{
  fsout.fill('0');
  fsout << hex;

  unsigned int tag_digits, index_digits;

  // Get the number of bits necessary to represent 'index'
  unsigned int temp = index_size*associativity;
  unsigned int bit_pos = 1;
  index_digits = 0;
  for (unsigned int i=0; i<(sizeof(ADDRESS)*8); i++) {
    if (temp & (bit_pos<<i)) break;
    index_digits++;
  } 

  // number of hexa digits for 'index' and 'tag' fields
  index_digits = (index_digits % 4) ? index_digits/4+1 : index_digits/4;
  tag_digits = (cache.m_tag_size % 4) ? cache.m_tag_size/4+1 : cache.m_tag_size/4;

  for (unsigned int i=0; i<index_size*associativity; i++) {
    fsout << "block["; fsout.width(index_digits);
    fsout << i << "]: (";

    cache.m_blocks[i].status->print(fsout);

    fsout << ") ";
    fsout.width(sizeof(ADDRESS)*2);
    fsout << cache.block_address(cache.m_blocks[i]) << " <";
    fsout.width(tag_digits);
    fsout  << cache.block_tag(cache.m_blocks[i]) << "> ";
      
    for (unsigned int j=0; j<block_size/sizeof(cpu_word); j++) {
      fsout.width(2*sizeof(cpu_word)); 
      fsout << (unsigned long long)*(cache.m_blocks[i].data+j);
      fsout << " ";
    }
    
    fsout << endl;

    if (associativity > 1 && associativity < index_size*associativity) 
      if (!((i+1) % associativity)) fsout << endl;
  }

  fsout << dec;
  fsout.fill(' ');


  cache.print_statistic(fsout);

  return fsout; 
}



#endif /* cache_bhv_h */
