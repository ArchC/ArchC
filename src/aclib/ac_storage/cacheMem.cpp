#include "cacheMem.h"
#include "cacheBlock.h"


cacheMem::~cacheMem()
{
	delete blocks;
}


void cacheMem::alocate_blocks(int index_size)
{
	blocks = new cacheBlock[nWay*index_size];
}

bool cacheMem::validate(uint32_t address, int index)
{
	if(index>MAXBLOCKS)
		return false;
	blocks[index].validate(address);
	return true;
}
bool cacheMem::checkValidation(uint32_t address, int cacheIndex)
{
	if(blocks[cacheIndex].address == address)
		return blocks[cacheIndex].checkValidation(address);
	return false;
}

void cacheMem::invalidate(uint32_t address, int cacheBlockIndex)
{

	for(int i=0; i<nWay;i++)
	{
		if(blocks[cacheBlockIndex+i].address == address){
			blocks[cacheBlockIndex+i].invalidate();
			i=nWay;
		}
	}
}
