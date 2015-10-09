#ifndef _AC_CACHE_H_INCLUDED_
#define _AC_CACHE_H_INCLUDED_

#include "ac_cache_bhv.H"
#include "ac_cache_trace.H"
#define HAVE_DIR 1
#ifdef HAVE_DIR
#include "Dir.h"
#endif
#define MEM_SIZE_ 0x20000000


struct write_back_state {
	bool valid;
	bool dirty;
	write_back_state() : valid(false), dirty(false) {}
	bool is_invalid() {
		return (!valid);
	}
	bool is_dirty() {
		return (valid && dirty);
	}
	void set_valid() {
		valid = true;
		dirty = false;
	}
	void set_invalid() {
		valid = false;
		dirty = false;
	}
	void set_dirty() {
		dirty = true;
	}
	void print(std::ostream &fsout) {
		if (valid) fsout << 'V';
		else fsout << 'I';
		if (dirty) fsout << 'D';
		else fsout << 'C';
	}
};

struct write_through_state {
	bool valid;
	write_through_state() : valid(false) {}
	bool is_invalid() {
		return (!valid);
	}
	void set_valid() {
		valid = true;
	}
	void set_invalid() {
		valid = false;
	}
	void print(std::ostream &fsout) {
		if (valid) fsout << 'V';
		else fsout << 'I';
	}
};

struct cache_statistics {
	unsigned long long read_hit;
	unsigned long long read_miss;
	unsigned long long write_hit;
	unsigned long long write_miss;
	unsigned long long evictions;
};

template <
	unsigned index_size,
	unsigned block_size,
	unsigned associativity,
	typename cpu_word,
	typename backing_store,
	typename replacement_policy,
	typename address = unsigned
>
class ac_write_back_cache {
	cache_bhv<index_size, block_size, associativity, cpu_word, address, 
	          write_back_state, replacement_policy> cache;
	backing_store &memory;
	ac_cache_trace *cache_trace;
	bool trace_active;
	
	int idCache;
	
 	void setId (int id)
    {
   		idCache = id;
    }
	int getId ()
    {
   		return idCache;
    }


	address byte_to_word(address a) {
		return a/sizeof(cpu_word);
	}
	address word_to_byte(address a) {
		return a*sizeof(cpu_word);
	}
	
	ac_write_back_cache(const ac_write_back_cache &, const int proc_id=-1);
	
	public:
	ac_write_back_cache(backing_store &memory_, const int proc_id=-1) : memory(memory_), trace_active(false),
  cache(proc_id) {

  		setId(proc_id);
		memory.setBlockSize (block_size);
	}
	
	~ac_write_back_cache() {
		if (trace_active) delete cache_trace;
	}
	
	void set_trace(std::ostream &o) {
		if (trace_active) delete cache_trace;
		cache_trace = new ac_cache_trace(o);
		trace_active = true;
	}
	

	  const cpu_word *read(address a, unsigned length) {

		//printf("\nAC_CACHE WB::read -> address=%x  length=%d", a, length);

		address b = byte_to_word(a);

		if(a >= MEM_SIZE_){
			a = a/block_size*block_size;
			const cpu_word *d = memory.read_block(a, sizeof(cpu_word));
			return d;
		}


		if (!cache.get_block_for_read(b)) {
			cache.get_available_block();
			if (cache.block_status().is_dirty()) {
				memory.write_block(word_to_byte(cache.block_address()),
									     cache.read_block(), block_size);
			}
			a = a/block_size*block_size;
			const cpu_word *d = memory.read_block(a, block_size);
			cache.write_block(d);
			cache.block_status().set_valid();
		}
		if (trace_active) cache_trace->add(trace_read, word_to_byte(b), length);
		return cache.read_block_single();
	}





	  void write(address a, const cpu_word *d, unsigned length) {

		  //printf("\nAC_CACHE WB::write -> address=%x  length=%d", a, length);


		address b = byte_to_word(a);

		if(a >= MEM_SIZE_){
			a = a/block_size*block_size;
			memory.write_block(a, d, sizeof(cpu_word));
			return;	
		}

		if (!cache.get_block_for_write(b)) {
			cache.get_available_block();
			if (cache.block_status().is_dirty()) {
				memory.write_block(word_to_byte(cache.block_address()),
					     cache.read_block(), block_size);
			}
			a = a/block_size*block_size;
			const cpu_word *d = memory.read_block(a, block_size);
			cache.write_block(d);
			cache.block_status().set_valid();
		}
		if (trace_active) cache_trace->add(trace_write, word_to_byte(b), length);
		cache.write_block_single(d, length);
		cache.block_status().set_dirty();
	}



	uint32_t get_size() {
		return memory.get_size();
	}
	
	void get_statistics(cache_statistics *statistics) {
		statistics->read_hit = cache.number_read_hit();
		statistics->read_miss = cache.number_read_miss();
		statistics->write_hit = cache.number_write_hit();
		statistics->write_miss = cache.number_write_miss();
		statistics->evictions = cache.number_block_eviction();
	}
	
	void print(std::ostream &fsout) {
		fsout << cache;
	}
	
	void print_statistics(ostream &out) {
		cache.print_statistic(out);
	}

  	void powersc_connect() {
   		cache.ps.powersc_connect();
  	}
};


template <
	unsigned index_size,
	unsigned block_size,
	unsigned associativity,
	typename cpu_word,
	typename backing_store,
	typename replacement_policy,
	typename address = unsigned
>
class ac_write_through_cache {
	cache_bhv<index_size, block_size, associativity, cpu_word, address,
	          write_through_state, replacement_policy> cache;
	backing_store &memory;
	ac_cache_trace *cache_trace;
	bool trace_active;
	int idCache;
	int ref;
	#ifdef HAVE_DIR
		static Dir dir;
	#endif
	
	
	void setId (int id)
    {
   		idCache = id;
    }
	int getId ()
    {
   		return idCache;
    }

	address byte_to_word(address a) {
		return a/sizeof(cpu_word);
	}
	address word_to_byte(address a) {
		return a*sizeof(cpu_word);
	}
	
	ac_write_through_cache(const ac_write_through_cache &, const int proc_id=-1);
	
	public:
	ac_write_through_cache(backing_store &memory_, const int proc_id=-1) : memory(memory_), trace_active(false),
  cache(proc_id) {

		setId(proc_id);
		memory.setBlockSize (block_size);
		ref =0;
		#ifdef HAVE_DIR
		if(getId() == 0)
			dir.start(associativity, index_size);
		#endif
	}
	~ac_write_through_cache() {
		if (trace_active) delete cache_trace;
	}

	void set_trace(std::ostream &o) {
		if (trace_active) delete cache_trace;
		cache_trace = new ac_cache_trace(o);
		trace_active = true;
	}
	
	const cpu_word *read(address a, unsigned length) {

		
			address b = byte_to_word(a);

			
			a = a/block_size*block_size;
			if(a >= MEM_SIZE_ ){

				const cpu_word *d = memory.read_block(a, length);
				return d;
			}
			
			int cacheIndex=0;
			bool cacheAnswer = !cache.get_block_for_read(b, &cacheIndex);
			uint32_t tag = (uint32_t) cache.get_tag(b); 
			bool cdir = false;
			#ifdef HAVE_DIR
				cdir = !(dir.checkValidation(getId(), tag, cacheIndex));
			#endif
			if ( cacheAnswer || cdir) {
				#ifdef HAVE_DIR
					if(!cacheAnswer){
						cache.invalidate(b);
						cache.memory_read_hit();
					}
				#endif
				cacheIndex = cache.get_available_block();
				const cpu_word *d = memory.read_block(a, block_size);
				#ifdef HAVE_DIR
					dir.validate(getId(), tag, cacheIndex);
				#endif
				cache.write_block(d);
				cache.block_status().set_valid();
			}

			if (trace_active) cache_trace->add(trace_read, word_to_byte(b), length);

			const cpu_word *cached = cache.read_block();

			return cache.read_block_single();
		}

		void write(address a, const cpu_word *d, unsigned length) {
   		    address b = byte_to_word(a);

			a = a/block_size*block_size;
			if(a >= MEM_SIZE_){

				
				memory.write_block(a, d, length);
				return;	
			}
			
			int cacheIndex=0;
			int cacheBlock=0;
			
			
			bool cacheAnswer = !cache.get_block_for_write(b, &cacheIndex, &cacheBlock);
			uint32_t tag = (uint32_t) cache.get_tag(b);		
			
			bool cdir2 = false;
			#ifdef HAVE_DIR
				cdir2 = !(dir.checkValidation(getId(), tag, cacheIndex));
			#endif
			if ( cacheAnswer || cdir2) {
				#ifdef HAVE_DIR
					if(!cacheAnswer){
						cache.invalidate(b);
						cache.memory_write_hit();
					}
				#endif
				cacheIndex = cache.get_available_block();
				const cpu_word *tmp_d = memory.read_block(a, block_size);
				#ifdef HAVE_DIR
					dir.validate(getId(), tag, cacheIndex);
				#endif
				cache.write_block(tmp_d);
				cache.block_status().set_valid();
			}
			if (trace_active) cache_trace->add(trace_write, word_to_byte(b), length);

			const cpu_word *cached = cache.read_block();


			cache.write_block_single(d, length);
            memory.write_block(word_to_byte(cache.block_address()), cache.read_block(),block_size);
			#ifdef HAVE_DIR
				dir.unvalidate(getId(), tag, cacheBlock);
			#endif 
			
	}
	
	void get_statistics(cache_statistics *statistics) {
		statistics->read_hit = cache.number_read_hit();
		statistics->read_miss = cache.number_read_miss();
		statistics->write_hit = cache.number_write_hit();
		statistics->write_miss = cache.number_write_miss();
		statistics->evictions = cache.number_block_eviction();
	}
	
	uint32_t get_size() {
		return memory.get_size();
	}

	void print(std::ostream &fsout) {
		fsout << cache;
	}
	
	void print_statistics(ostream &out) {
		cache.print_statistic(out);
	}
	void invalidate_address(uint32_t a){
	}
 	void powersc_connect() {
   		cache.ps.powersc_connect();
  	}
};
#ifdef HAVE_DIR
	template <
	unsigned index_size,
	unsigned block_size,
	unsigned associativity,
	typename cpu_word,
	typename backing_store,
	typename replacement_policy,
	typename address
> Dir ac_write_through_cache <index_size, block_size, associativity, cpu_word, backing_store, replacement_policy, address>::dir;		
#endif

#endif /* _AC_CACHE_H_INCLUDED_ */

