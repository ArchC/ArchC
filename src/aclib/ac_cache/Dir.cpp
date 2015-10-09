

#include "Dir.h"


Dir::Dir()
{
	started=0;
	totalOfCaches=MAX_NUMBER_OF_CACHES;
	index_size = MAX_INDEX_SIZE;
	// log.open("dirLog.txt", fstream::out);
	for(int i=0; i<totalOfCaches;i++){
		cacheMemVector[i].numberCache = i;
	}
	start(2, index_size);

}

void Dir::start(int n, int index)
{
	if(started != 0)
		return;
	started++;
	index_size = index;
	for(int i=0; i<totalOfCaches;i++){
		cacheMemVector[i].nWay = n;
		cacheMemVector[i].alocate_blocks(index_size);
	}
}

Dir::~Dir()
{
	//log.close();
}



bool Dir::validate(int numberCache, uint32_t address, int cacheAddress)
{
	
	cacheMemVector[numberCache].validate(address, cacheAddress);
	return true;

}


bool Dir::checkValidation(int numberCache, uint32_t address, int cacheIndex)
{
	if(cacheMemVector[numberCache].checkValidation(address, cacheIndex))
	{
		return true;
	}
	return false;
}

void Dir::unvalidate(int numberCache, uint32_t address, int cacheBlockIndex)
{

	for(int i=0; i<totalOfCaches;i++)
		if(cacheMemVector[i].numberCache != numberCache)
		{
			cacheMemVector[i].invalidate(address, cacheBlockIndex);
		}
}


void Dir::checkNumberCache(int numberCache)
{
	for(int i=0; i<totalOfCaches;i++)
	{
		if(numberCache == cacheMemVector[i].numberCache)
			return;
	}
}
