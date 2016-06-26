#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#include <windows.h>
#include <vector>

#include "commonvulkan.h"
#include "surface.h"
#include "commonwindows.h"
#include "camera.h"

static const char* EXE_NAME = "VulkanTriangle";
static const uint32_t VERTEX_BUFFER_BIND_ID = 0;
static const float CAMERA_SPEED = 0.001f;
#define VALIDATION_LAYERS false
#define VALIDATION_MESSAGES false
#define DEBUGGING false



//vertex data store don ram
struct Vertex
{
	float pos[3];
	float col[3];
};


//vertex data stored on the gpu ram
struct VertexBuffer
{
	//vertex placement data
	std::vector<Vertex> vPos;
	VkBuffer vBuffer;
	VkDeviceMemory vMemory;
	VkPipelineVertexInputStateCreateInfo viInfo;
	std::vector<VkVertexInputBindingDescription> vBindingDesc;
	std::vector<VkVertexInputAttributeDescription> vBindingAttr;

	//vertex index data
	std::vector<uint32_t> iPos;
	int iCount;
	VkBuffer iBuffer;
	VkDeviceMemory iMemory;

};



//main struct, pretty much holds everything
struct MainMemory
{
	HWND consoleHandle;
	WindowInfo windowInfo;
	TimerInfo timerInfo;

	InputInfo input;

	VkInstance vkInstance;
	SurfaceInfo surfaceInfo;
	PhysDeviceInfo physDeviceInfo;
	DebugInfo debugInfo;
	DeviceInfo deviceInfo;

	VkCommandBuffer textureCmdBuffer;
	VertexBuffer vertexBuffer;

	Camera camera;
	PipelineInfo pipelineInfo;



};