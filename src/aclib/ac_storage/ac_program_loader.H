#ifndef _AC_PROGRAM_LOADER_H_INCLUDED_
#define _AC_PROGRAM_LOADER_H_INCLUDED_

#include <ac_arch_ref.H>
#include <ac_utils.H>
#include <stdlib.h>

template <typename storage_t, typename ac_word, typename ac_Hword>
class ac_program_loader : public ac_arch_ref<ac_word, ac_Hword> {
	storage_t &storage;
	// Compatibility with ac_memport::write()
	inline void write(uint32_t address, ac_word datum) {
		ac_word aux_word = datum;
		if (!this->ac_mt_endian) {
			aux_word = byte_swap(datum);
		}
		ac_ptr ptr = &aux_word;
		storage.write(ptr, address, sizeof(ac_word) * 8, 1);
	}

	public:
	explicit ac_program_loader(ac_arch<ac_word, ac_Hword>& arch, storage_t &storage_) :
		ac_arch_ref<ac_word, ac_Hword>(arch), storage(storage_) {}

	///Method to load device content from a file.
	/// Use local Data array
	// Copied from ac_memport::load()
	inline void load(char* file) {

		ifstream input;
		string read;
		string word;
		istringstream line;
		unsigned text_size=0;
		bool is_addr, is_text=0, first_addr=1;
		long long data;
		unsigned int  addr=0;
		unsigned char* Data;

		Data = new unsigned char[storage.get_size()];

		//Try to read as ELF first
		if (ac_load_elf<ac_word, ac_Hword>(*this, file, Data, storage.get_size(),
						   this->ac_heap_ptr, this->ac_start_addr,
						   this->ac_mt_endian) == EXIT_SUCCESS) {
			//init decode cache and return
			if(!this->dec_cache_size) this->dec_cache_size = this->ac_heap_ptr;
			storage.write(Data, 0, 32, (this->ac_heap_ptr)/4);
			delete[] Data;
			return;
		}

		// Looking for initialization file.
		input.open(file);
		if(!input){
			AC_ERROR("Could not open input file:" << file);
			AC_ERROR("Simulation aborted.");
			exit(1);
		}
		else{
			while( !input.eof() ){
				line.clear();
				getline(input, read);
				line.str(read);

				is_addr = 1;

				//Processing line
				while(line >> word){

					//Processing word
					if( is_addr ) {
						addr = strtol(word.c_str(), NULL, 16);
						is_addr = 0;
						if( is_text && first_addr ) {
							first_addr = 0;
							this->ac_start_addr = addr;
						}
					}
					else {
						if(is_text) text_size++;
						data = strtoll( word.c_str(), NULL, 16);
						write(addr,(ac_word)data);
						addr+= sizeof(ac_word);
					}
				}
			}
		}
		if(!this->dec_cache_size)
		this->dec_cache_size = addr;

		input.close();
	}

	uint32_t get_size() const {
		return storage.get_size();
	};
};
;
#endif /* _AC_PROGRAM_LOADER_H_INCLUDED_ */
