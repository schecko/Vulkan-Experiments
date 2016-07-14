#pragma once

#include <string>
#include <vulkan.h>
#include <iostream>
#include <windows.h> //todo make assert not windows dependant? idk

#define ENGINE_DEBUG true
#define VALIDATION_MESSAGES true


namespace Cy
{
	//helper macros for annoying function pointers
#define GET_VULKAN_FUNCTION_POINTER_INST(inst, function)							\
{																					\
	surfaceInfo.##function = (PFN_vk##function) vkGetInstanceProcAddr(inst, "vk"#function);		\
	Assert(surfaceInfo.##function != nullptr, "could not find function "#function);				\
}																					\

#define GET_VULKAN_FUNCTION_POINTER_DEV(dev, function)								\
{																					\
	function = (PFN_vk##function) vkGetDeviceProcAddr(dev, "vk"#function);			\
	Assert(function != nullptr, "could not find function "#function);				\
}																					\


	inline void Message(std::string message)
	{
		message += "\n";
#if 1
		fprintf(stdout, message.c_str());
#else
		std::cout << message.c_str() << std::endl;
#endif
	}

	inline void Message(uint64_t message)
	{
		std::string msg = std::to_string(message) + "\n";
		fprintf(stdout, msg.c_str());
	}

	inline void Message(double message)
	{
		std::string msg = std::to_string(message) + "\n";
		fprintf(stdout, msg.c_str());
	}

	//assert the test is true, if test is false the console is moved to the focus,
	//the message is printed out and execution halts using std::cin rather than abort() or nullptr errors
	inline void Assert(bool test, std::string message)
	{
#if ENGINE_DEBUG
		char x;
		if (test == false)
		{
			HWND consoleHandle = GetConsoleWindow();
			ShowWindow(consoleHandle, SW_SHOW);
			SetFocus(consoleHandle);
			Message(message);
			std::cin >> x;
			//ShowWindow(consoleHandle, SW_HIDE);
		}
#endif
	}

	inline void Assert(VkResult test, std::string message)
	{
		Assert(test == VK_SUCCESS, message);
	}

	inline uint32_t SafeTruncate(uint64_t value)
	{
		bool safe = !((value >> 32) && 0xffffffff);
		Assert(safe, "truncate NOT SAFE");
		return (uint32_t)value;
	}
	inline uint16_t SafeTruncate(uint32_t value)
	{
		bool safe = !((value >> 16) && 0xffff);
		Assert(safe, "truncate NOT SAFE");
		return (uint16_t)value;
	}
	inline uint8_t SafeTruncate(uint16_t value)
	{
		bool safe = !((value >> 8) && 0xff);
		Assert(safe, "truncate NOT SAFE");
		return (uint8_t)value;
	}
	inline int32_t SafeTruncate(int64_t value)
	{
		if(value >> 63)
		{
			bool safe = !(!(value >> 32) && 0xffffffff);
			Assert(safe, "truncate NOT SAFE");
			return (int32_t)value;
		}
		else
		{
			bool safe = !((value >> 32) && 0xffffffff);
			Assert(safe, "truncate NOT SAFE");
			return (int32_t)value;
		}
	}
	inline int16_t SafeTruncate(int32_t value)
	{
		if (value >> 31)
		{
			bool safe = !(!(value >> 16) && 0xffff);
			Assert(safe, "truncate NOT SAFE");
			return (int16_t)value;
		}
		else
		{
			bool safe = !((value >> 16) && 0xffff);
			Assert(safe, "truncate NOT SAFE");
			return (int16_t)value;
		}
	}
	inline int8_t SafeTruncate(int16_t value)
	{
		if (value >> 15)
		{
			bool safe = !(!(value >> 8) && 0xff);
			Assert(safe, "truncate NOT SAFE");
			return (int8_t)value;
		}
		else
		{
			bool safe = !((value >> 8) && 0xff);
			Assert(safe, "truncate NOT SAFE");
			return (int8_t)value;
		}
	}
}
