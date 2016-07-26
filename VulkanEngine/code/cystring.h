#pragma once

#include <stdint.h>

namespace Cy
{
	struct String
	{
		uint32_t size;
		const char* data;
	};


	String operator+(String s1, String s2);

}