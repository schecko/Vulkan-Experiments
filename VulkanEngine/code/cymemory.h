#pragma once

#include <stdint.h>

namespace Cy
{

	struct Memory
	{
		uint32_t capacity;
		void* data;
	};

	struct ThreadPool
	{
		//each thread limited to 3 links of a link list
		Memory memory[3];
		uint32_t space[2]; //space for mutex handles etc
	};

	struct MemoryPool
	{
		Memory memory;
		ThreadPool threadPool[16];

	};


	void InitPool(uint32_t* data, uint32_t size, uint32_t numThreads);
}
