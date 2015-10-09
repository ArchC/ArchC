#ifndef DIR_H
#define DIR_H

#define MAX_NUMBER_OF_CACHES 64
#define MAX_INDEX_SIZE 1024

#include "cacheMem.h"

class Dir
{
	public:
		//fstream log;
		int totalOfCaches, index_size, started;
		cacheMem cacheMemVector[MAX_NUMBER_OF_CACHES];
		Dir();
		Dir(int p){totalOfCaches=p;}
		virtual ~Dir();
		void start(int, int);
		bool checkValidation(int, uint32_t, int);
		void checkNumberCache(int);
		bool validate(int, uint32_t, int);
		void unvalidate(int, uint32_t, int);

	protected:
	private:
};

#endif // DIR_H
