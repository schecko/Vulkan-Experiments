
#include "cymemory.h"
#include <cstdint>


void InitPool(uint32_t* data, uint32_t size, uint32_t numThreads)
{
	data[0] = size;
	data[1] = numThreads;

	for (uint32_t i = 0; i < numThreads; i++)
	{
		//data[2 + i] = mutex info?
	}

}
