#ifndef _AC_CACHE_IF_H_INCLUDED_
#define _AC_CACHE_IF_H_INCLUDED_

#include "ac_inout_if.H"

template <typename ac_word, typename ac_Hword, typename cache_t>
class ac_cache_if : public ac_inout_if {
	cache_t &cache;
	public:
	explicit ac_cache_if(cache_t &c) : cache(c) {}
	virtual ~ac_cache_if() {}
	/** 
	* Reads a single word.
	* 
	* @param buf Buffer into which the word will be copied.
	* @param address Address from where the word will be read.
	* @param wordsize Word size in bits.
	* 
	*/
	virtual void read(ac_ptr buf, uint32_t address, int wordsize) {
            
		//printf("\nAC_CACHE_IF::read -> address=%x", address);
		const ac_word *w = cache.read(address, sizeof(ac_word));
		const uint8_t *b;
		const ac_Hword *x;
		ac_Hword *h;
		ac_word *y;
		uint32_t offset;
		switch(wordsize) {
		case 8:
			b = (const uint8_t *)w;
			offset = address%sizeof(ac_word);

			*buf.ptr8 = b[offset];
			//printf("\nAC_CACHE_IF::read case 8");
			break;
		case 8*sizeof(ac_Hword):
			x = (const ac_Hword *)w;
			offset = address%sizeof(ac_word)/sizeof(ac_Hword);
			h = (ac_Hword *)buf.ptr8;

			*h = x[offset];
			//printf("\nAC_CACHE_IF::read case 16");
			break;
		case 8*sizeof(ac_word):
			y = (ac_word *)buf.ptr8;
		    *y = *w;
			//printf("\nAC_CACHE_IF::read case 32");

			break;
		default:
			abort();
		}
	}

	/** 
	* Reads multiple words.
	* 
	* @param buf Buffer into which the words will be copied.
	* @param address Address from where the words will be read.
	* @param wordsize Word size in bits.
	* @param n_words Number of words to be read.
	* 
	*/
	virtual void read(ac_ptr buf, uint32_t address,
		    int wordsize, int n_words) {
		abort();
	}
	
	/** 
	* Writes a single word.
	* 
	* @param buf Buffer from which the word will be copied.
	* @param address Address to where the word will be written.
	* @param wordsize Word size in bits.
	*
	*/
	virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize) {
            

		//printf("\nAC_CACHE_IF::write -> address=%x", address);

		ac_word *w = (ac_word *)buf.ptr8;


		uint8_t *b;
		ac_word r;
		ac_Hword *h;

		uint32_t offset;
		// FIXME: cache only supports word access
		switch(wordsize) {
		case 8:

			r = *cache.read(address, sizeof(ac_word));
			b = (uint8_t *)&r;
			offset = address%sizeof(ac_word);
			b[offset] = *buf.ptr8;
			cache.write(address, &r, sizeof(ac_word));
			break;
		case 8*sizeof(ac_Hword):

			r = *cache.read(address, sizeof(ac_word));
			h = (ac_Hword *)&r;
			offset = address%sizeof(ac_word)/sizeof(ac_Hword);
			h[offset] = *(ac_Hword *)buf.ptr8;
			cache.write(address, &r, sizeof(ac_word));
			break;
		case 8*sizeof(ac_word):
		    cache.write(address, w, sizeof(ac_word));
			break;
		default:
			abort();
		}
	}

	/** 
	* Writes multiple words.
	* 
	* @param buf Buffer from which the words will be copied.
	* @param address Address to where the words will be written.
	* @param wordsize Word size in bits.
	* @param n_words Number of words to be written.
	* 
	*/
	virtual void write(ac_ptr buf, uint32_t address,
		     int wordsize, int n_words) {
		abort();
	}




	virtual void read(ac_ptr buf, uint32_t address, int wordsize,sc_core::sc_time &time_info) {

			this->read(buf,address,wordsize);
		}

		/**
		* Reads multiple words.
		*
		* @param buf Buffer into which the words will be copied.
		* @param address Address from where the words will be read.
		* @param wordsize Word size in bits.
		* @param n_words Number of words to be read.
		*
		*/
		virtual void read(ac_ptr buf, uint32_t address,
			    int wordsize, int n_words,sc_core::sc_time &time_info) {
			abort();
		}

		/**
		* Writes a single word.
		*
		* @param buf Buffer from which the word will be copied.
		* @param address Address to where the word will be written.
		* @param wordsize Word size in bits.
		*
		*/
		virtual void write(ac_ptr buf, uint32_t address,
			     int wordsize,sc_core::sc_time &time_info) {


			this->write(buf,address,wordsize);
		}

		/**
		* Writes multiple words.
		*
		* @param buf Buffer from which the words will be copied.
		* @param address Address to where the words will be written.
		* @param wordsize Word size in bits.
		* @param n_words Number of words to be written.
		*
		*/
		virtual void write(ac_ptr buf, uint32_t address,
			     int wordsize, int n_words,sc_core::sc_time &time_info) {
			abort();
		}





	virtual std::string get_name() const {
		return "cache";
	}

	virtual uint32_t get_size() const {
		return cache.get_size();
	}

	/** 
	* Locks the device.
	* 
	*/
	virtual void lock() {
	}

	/** 
	* Unlocks the device.
	* 
	*/
	virtual void unlock() {
	}
};
;
#endif /* _AC_CACHE_IF_H_INCLUDED_ */
