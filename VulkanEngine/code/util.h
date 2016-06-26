#pragma once

#include <string>
#include <vulkan.h>
#include <iostream>

//helper macros for annoying function pointers
#define GET_VULKAN_FUNCTION_POINTER_INST(inst, function)							\
{																					\
	function = (PFN_vk##function) vkGetInstanceProcAddr(inst, "vk"#function);		\
	Assert(function != nullptr, "could not find function "#function);				\
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
#if DEBUGGING == true
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

