#include "cacheMem.h"
#include "cacheBlock.h"
#include <iostream>
using namespace std;

void cacheBlock::validate(uint32_t m_address)
{
	address = m_address;
	validated = true;
	state = 'S';
	//cout << "validou-se address: " << address<<endl;
}

void cacheBlock::setStateBlock(char new_state)
{
	state = new_state;
}

bool cacheBlock::checkValidation(uint32_t m_address)
{
    //cout << " address: " << address;
    if(state == 'I')
        return false;
	if((m_address == address) && state != 'I'){
        //cout << " retornou true"<<endl;
		return true;
	}
    //cout << " retornou false"<<endl;
	return false;
}


void cacheBlock::invalidate()
{
	validated = false;
	state = 'I';
    address = -1;
}


cacheBlock::cacheBlock()
{
	address = -1;
	validated = false;
	state = 'I';
}

