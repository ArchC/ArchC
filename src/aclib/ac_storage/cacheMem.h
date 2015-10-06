#ifndef CACHEMEMORYLIST_H
#define CACHEMEMORYLIST_H
#include "cacheBlock.h"
#define MAXBLOCKS 1024
class cacheMem
{
	public:
		int numberCache;
		int nWay;
		int cacheModIndex;
		cacheMem(){};
		void alocate_blocks(int index_size);
		cacheMem(int n){nWay=n;};
		cacheBlock *blocks;
		bool validate(uint32_t, int);
		bool checkValidation(uint32_t, int);
		bool invalidate(uint32_t, int);
		bool readSetState(uint32_t, int);
		void writeSetState(uint32_t, int);

		virtual ~cacheMem();
	protected:
	private:
};

#endif // CACHEMEMORYLIST_H
