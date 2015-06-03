#include "cacheMem.h"
#include "cacheBlock.h"

void cacheBlock::validate(uint32_t m_address)
{
	address = m_address;
	validated = true;
}

bool cacheBlock::checkValidation(uint32_t m_address)
{
	if((m_address == address) && validated){
		return true;
	}
	return false;
}

void cacheBlock::invalidate()
{
	validated = false;
}

cacheBlock::cacheBlock()
{
	address = -1;
	validated = false;
}

