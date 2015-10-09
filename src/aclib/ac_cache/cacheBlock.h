
#ifndef CACHEBLOCK_H
#define CACHEBLOCK_H
#include <stdint.h>

class cacheBlock
{
	public:

		uint32_t address;
		bool validated;
		char state;
		void validate(uint32_t);
		void invalidate();
		bool checkValidation(uint32_t);
		void setStateBlock(char);
		cacheBlock();
	protected:
	private:
};

#endif // CACHEBLOCK_H
