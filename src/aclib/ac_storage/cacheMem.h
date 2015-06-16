#ifndef CACHEMEMORYLIST_H
#define CACHEMEMORYLIST_H

#define MAXBLOCKS 1024

#include "cacheBlock.h"


class cacheMem
{
	public:
		int numberCache;
		int nWay;
		cacheMem(){};
		void alocate_blocks(int index_size);
		cacheMem(int n){nWay=n;};
		cacheBlock *blocks;
		bool validate(uint32_t, int);
		bool checkValidation(uint32_t, int);
		void invalidate(uint32_t, int);


		virtual ~cacheMem();
	protected:
	private:
};

#endif // CACHEMEMORYLIST_H
