#ifndef _AC_MEM_H_INCLUDED_
#define _AC_MEM_H_INCLUDED_

#include <cstdlib>
#include <cstring>
#include <ostream>

#include <ac_ptr.H>

template <
	unsigned length,
	typename cpu_word = unsigned,
	typename address = unsigned
>
class ac_mem {
	cpu_word data[length];
	public:
	ac_mem() {
		clear();
	}
	
	void clear() {
		std::memset(data, 0, length*sizeof(cpu_word));
	}	
	
	address byte_to_word(address a) {
		return a/sizeof(cpu_word);
	}
	
	const cpu_word *read(address a, unsigned l) {
		a = byte_to_word(a);
		return &data[a];
	}
	

	// just for compatibility with ac_cache
	const cpu_word *read_block(address a, unsigned l) {
			a = byte_to_word(a);
			return &data[a];
		}


	void write(address a, const cpu_word *d, unsigned l) {
		a = byte_to_word(a);
		l = byte_to_word(l);
		unsigned max_length = length-a;
		if (l > max_length) std::abort();
		
		for (unsigned i = 0; i < l; i++) {
			data[a+i] = d[i];
		}
	}
	
	// just for compatibility with ac_cache
	void write_block(address a, const cpu_word *d, unsigned l) {
			a = byte_to_word(a);
			l = byte_to_word(l);
			unsigned max_length = length-a;
			if (l > max_length) std::abort();

			for (unsigned i = 0; i < l; i++) {
				data[a+i] = d[i];
			}
		}


	// Compatibility with ac_inout_if::write()
	void write(ac_ptr buf, uint32_t address_, int wordsize, int n_words) {
		cpu_word *d = (cpu_word *)buf.ptr8;
		unsigned l = wordsize/8*n_words;
		address a = address_;
		write(a, d, l);
	}

	// Compatibility with ac_inout_if::get_size()
	uint32_t get_size() const {
		return length*sizeof(cpu_word);
	}


	void print(std::ostream &o) {
		unsigned i, j;
		o << std::hex;
		o.fill('0');
		for (i = 0; i < length/4; i++) {
			o << '(';
			o.width(2*sizeof(address));
			o << i*4 << ") ";
			for (j = 0; j < 4; j++) {
				o.width(2*sizeof(cpu_word));
				o << data[4*i+j] << ' ';
			}
			o << '\n';
		}
	}
	
};

#endif /* _AC_MEM_H_INCLUDED_ */

