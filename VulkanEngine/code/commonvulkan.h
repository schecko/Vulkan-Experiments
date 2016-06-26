#pragma once


#include <vulkan.h>
#include <vector>
#include "surface.h"



//TODO dont really know
//TODO rename?
 struct DepthStencil
{
	VkDeviceMemory mem;
	VkImage image;
	VkImageView view;
};

//temporary buffers and memory used to move the vertex data to the gpu
struct StagingBuffer
{
	VkDeviceMemory memory;
	VkBuffer buffer;
};

struct StagingBuffers
{
	StagingBuffer vertices;
	StagingBuffer indices;
};


struct PhysDeviceInfo
{
	VkPhysicalDevice physicalDevice;
	uint32_t renderingQueueFamilyIndex;
	VkFormat supportedDepthFormat;

	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceMemoryProperties memoryProperties;
};

struct DeviceInfo
{
	VkDevice device;
	VkQueue queue;
	VkCommandPool cmdPool;

	//TODO better name?
	VkSubmitInfo submitInfo;

	VkSemaphore presentComplete;
	VkSemaphore renderComplete;

	VkCommandBuffer setupCmdBuffer;
	VkCommandBuffer prePresentCmdBuffer;
	VkCommandBuffer	postPresentCmdBuffer;

	std::vector<VkCommandBuffer> drawCmdBuffers;
	std::vector<VkFramebuffer> frameBuffers;

	DepthStencil depthStencil; //TODO would this be better in the surfaceinfo struct?
};

struct PipelineInfo
{
	VkRenderPass renderPass;
	VkPipelineCache pipelineCache;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	std::vector<VkShaderModule> shaderModules;
	VkPipeline pipeline;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;

};

struct DebugInfo
{
	std::vector<const char*> instanceLayerList;
	std::vector<const char*> instanceExtList;
	std::vector<const char*> deviceLayerList;
	std::vector<const char*> deviceExtList;
	VkDebugReportCallbackEXT debugReport;
};

void CreateDebugCallback(VkInstance vkInstance, VkDebugReportCallbackEXT* debugReport);

VkDescriptorSetLayout NewDescriptorSetLayout(VkDevice logicalDevice, VkDescriptorType type, VkShaderStageFlags flags);

VkPipelineLayout NewPipelineLayout(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout);

VkInstance NewVkInstance(const char* appName, std::vector<const char*>* instanceLayers, std::vector<const char*>* instanceExts);

std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance vkInstance, uint32_t* gpuCount);

VkDevice NewLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t renderingQueueFamilyIndex, std::vector<const char*> deviceLayers, std::vector<const char*> deviceExts);

VkFormat GetSupportedDepthFormat(VkPhysicalDevice physicalDevice);

VkSemaphore NewSemaphore(VkDevice logicalDevice);

VkCommandPool NewCommandPool(VkDevice logicalDevice, uint32_t renderingAndPresentingIndex);

VkCommandBuffer NewSetupCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool);

void SetImageLayout(VkCommandBuffer cmdBuffer,
	VkImage image,
	VkImageAspectFlags aspectMask,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout);

std::vector<VkLayerProperties> GetInstalledVkLayers();

std::vector<VkLayerProperties> GetInstalledVkLayers(VkPhysicalDevice physicalDevice);

VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObj,
	size_t location,
	int32_t msgCode,
	const char* layerPrefix,
	const char* msg,
	void* data);

VkCommandBuffer NewCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool);

std::vector<VkCommandBuffer> NewCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool, uint32_t numBuffers);

void GetMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkFlags properties, uint32_t* typeIndex);

void setupDepthStencil(DeviceInfo* deviceInfo,
	const PhysDeviceInfo* physDeviceInfo,
	uint32_t width,
	uint32_t height);

VkRenderPass NewRenderPass(VkDevice logicalDevice, VkFormat surfaceColorFormat, VkFormat depthFormat);

VkPipelineCache NewPipelineCache(VkDevice logicalDevice);

std::vector<VkFramebuffer> NewFrameBuffer(VkDevice logicalDevice,
	std::vector<VkImageView>* surfaceViews,
	VkRenderPass renderPass,
	VkImageView depthStencilView,
	uint32_t numBuffers,
	uint32_t width,
	uint32_t height);

void FlushSetupCommandBuffer(DeviceInfo* deviceInfo);

VkBuffer NewBuffer(VkDevice logicalDevice, uint32_t bufferSize, VkBufferUsageFlags usageBits);

StagingBuffer AllocBindDataToGPU(VkDevice logicalDevice, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t dataSize, void* dataTobind, VkBuffer* buffer, VkDeviceMemory* memory);

VkMemoryAllocateInfo NewMemoryAllocInfo(VkDevice logicalDevice, VkPhysicalDeviceMemoryProperties memoryProperties, VkBuffer buffer, uint32_t desiredAllocSize, VkFlags desiredMemoryProperties);

VkPipelineShaderStageCreateInfo NewShaderStageInfo(VkDevice logicalDevice, PipelineInfo* pipelineInfo, uint32_t* data, uint32_t dataSize, VkShaderStageFlagBits stage);

void DestroyPipelineInfo(VkDevice device, PipelineInfo* pipelineInfo);

void DestroyDeviceInfo(DeviceInfo* deviceInfo);

void DestroyDebugInfo(VkInstance vkInstance, DebugInfo* debugInfo);

void DestroyInstance(VkInstance vkInstance);