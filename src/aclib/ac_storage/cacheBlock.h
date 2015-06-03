
#ifndef CACHEBLOCK_H
#define CACHEBLOCK_H

#include <stdio.h>
#include <stdint.h>
#include "string.h"
#include <fstream>
#include <iostream>
using namespace std;

class cacheBlock
{
	public:

		uint32_t address;
		bool validated;
		void validate(uint32_t);
		void invalidate();
		bool checkValidation(uint32_t);
		cacheBlock();
	protected:
	private:
};

#endif // CACHEBLOCK_H
