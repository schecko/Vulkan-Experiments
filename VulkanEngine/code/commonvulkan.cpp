#include "commonvulkan.h"
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>
#include <vector>
#include "util.h"


namespace Cy
{
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT = nullptr;
	PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallbackEXT = nullptr;

	void CreateDebugCallback(VkInstance vkInstance, VkDebugReportCallbackEXT* debugReport)
	{
		VkDebugReportCallbackCreateInfoEXT debugCreateInfo = {};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		debugCreateInfo.pfnCallback = VkDebugCallback;
		debugCreateInfo.flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
			VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		VkResult error = CreateDebugReportCallbackEXT(vkInstance, &debugCreateInfo, nullptr, debugReport);
		Assert(error == VK_SUCCESS, "could not create vulkan error callback");

	}

	VKAPI_ATTR VkBool32 VKAPI_CALL VkDebugCallback(VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t srcObj,
		size_t location,
		int32_t msgCode,
		const char* layerPrefix,
		const char* msg,
		void* data)
	{
#if VALIDATION_MESSAGES == true
		std::string reportMessage;
		if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		{
			reportMessage += "DEBUG: ";

		}
		if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			reportMessage += "WARNING: ";

		}
		if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		{
			reportMessage += "PERFORMANCE WARNING: ";
		}
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			reportMessage += "ERROR: ";

		}
		if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		{

			reportMessage += "INFORMATION: ";
		}

		reportMessage += "@[";
		reportMessage += layerPrefix;
		reportMessage += "] ";
		reportMessage += msg;
		Message(msg);
#endif
		if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{

			Assert(0, "error in VK");
			return true;
		}

		return false;
	}

	VkDescriptorSetLayout NewDescriptorSetLayout(VkDevice logicalDevice, VkDescriptorType type, VkShaderStageFlags flags)
	{
		VkDescriptorSetLayoutBinding lBind = {};
		lBind.descriptorType = type;
		lBind.descriptorCount = 1;
		lBind.stageFlags = flags;
		lBind.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo dlInfo = {};
		dlInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		dlInfo.bindingCount = 1;
		dlInfo.pBindings = &lBind;

		VkDescriptorSetLayout descriptorSetLayout;


		VkResult error = vkCreateDescriptorSetLayout(logicalDevice, &dlInfo, nullptr, &descriptorSetLayout);
		Assert(error, "could not create descriptor set layout");
		return descriptorSetLayout;
	}

	VkPipelineLayout NewPipelineLayout(VkDevice logicalDevice, VkDescriptorSetLayout descriptorSetLayout)
	{
		VkPipelineLayout pipelinelayout;
		VkPipelineLayoutCreateInfo plInfo = {};
		plInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		plInfo.setLayoutCount = 1;
		plInfo.pSetLayouts = &descriptorSetLayout;
		VkResult error = vkCreatePipelineLayout(logicalDevice, &plInfo, nullptr, &pipelinelayout);
		Assert(error, "could not create pipeline layout");
		return pipelinelayout;
	}


	VkInstance NewVkInstance(const char* appName, std::vector<const char*>* instanceLayers, std::vector<const char*>* instanceExts)
	{

		VkResult error;
		VkInstance vkInstance;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.pEngineName = "VulkanEngine";
		appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 17);
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 17);

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;

		if (instanceLayers->size() > 0)
		{
			instanceCreateInfo.enabledLayerCount = (uint32_t)instanceLayers->size();
			instanceCreateInfo.ppEnabledLayerNames = instanceLayers->data();
		}

		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExts->size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExts->data();



		error = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);

		Assert(error == VK_SUCCESS, "could not create instance of vulkan");

		if (instanceLayers->size() > 0 && error == VK_SUCCESS)
		{
			GET_VULKAN_FUNCTION_POINTER_INST(vkInstance, CreateDebugReportCallbackEXT);
			GET_VULKAN_FUNCTION_POINTER_INST(vkInstance, DestroyDebugReportCallbackEXT);
		}

		return vkInstance;
	}


	VkInstance NewVkInstance(const char* appName, VkInclude features)
	{

		std::vector<VkExtensionProperties> extProps = GetInstalledVkExtensions();
		std::vector<VkLayerProperties> layerProps = GetInstalledVkLayers();
		std::vector<const char*> extNames;
		std::vector<const char*> layerNames;
		if (features > VkInclude::nothing)
		{
			switch (features)
			{
				//NOTE: vktrace layer broken right now.....
				/*case VkInclude::everything:
					{
						extNames.resize(extProps.size());
						layerNames.resize(layerProps.size());
						for (uint32_t i = 0; i < layerProps.size(); i++)
						{
							layerNames[i] = layerProps[i].layerName;
						}
						for (uint32_t i = 0; i < extProps.size(); i++)
						{
							extNames[i] = extProps[i].extensionName;
						}
					}
					break;*/
				case VkInclude::minimalBoth:
					{
						layerNames.push_back("VK_LAYER_LUNARG_api_dump");
						layerNames.push_back("VK_LAYER_LUNARG_core_validation");
						layerNames.push_back("VK_LAYER_LUNARG_device_limits");
						layerNames.push_back("VK_LAYER_LUNARG_image");
						layerNames.push_back("VK_LAYER_LUNARG_object_tracker");
						layerNames.push_back("VK_LAYER_LUNARG_parameter_validation");
						layerNames.push_back("VK_LAYER_LUNARG_screenshot");
						layerNames.push_back("VK_LAYER_LUNARG_swapchain");
						layerNames.push_back("VK_LAYER_GOOGLE_threading");
						layerNames.push_back("VK_LAYER_GOOGLE_unique_objects");
						//layerNames.push_back("VK_LAYER_LUNARG_vktrace"); //trace broken right now
						layerNames.push_back("VK_LAYER_RENDERDOC_Capture");
						layerNames.push_back("VK_LAYER_NV_optimus");
						layerNames.push_back("VK_LAYER_VALVE_steam_overlay");
						layerNames.push_back("VK_LAYER_LUNARG_standard_validation");

						//required for function GetPhysicalDeviceSurfaceSupportKHR
						extNames.push_back("VK_KHR_surface");
						//required for function vkCreateWin32SurfaceKHR 
						extNames.push_back("VK_KHR_win32_surface");
						//required for createdebugreportcallback function
						extNames.push_back("VK_EXT_debug_report");
					}
					break;
				case VkInclude::onlyExtensions:
					{
						Assert(((uint32_t)features & (uint32_t)VkInclude::onlyExtensions) == (uint32_t)features, "VkInclude::onlyExtensions cannot be used with other layer and extension options");
						//required for function GetPhysicalDeviceSurfaceSupportKHR
						extNames.push_back("VK_KHR_surface");
						//required for function vkCreateWin32SurfaceKHR 
						extNames.push_back("VK_KHR_win32_surface");
						//required for createdebugreportcallback function
						extNames.push_back("VK_EXT_debug_report");
					}
					break;
				default:
					{
						Assert(0, "vkinclude " + std::to_string((uint32_t)features) + "not implemented yet");
					}
					break;
			}
		}

		VkResult error;
		VkInstance vkInstance;

		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName;
		appInfo.pEngineName = "VulkanEngine";
		appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 17);
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 17);

		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.pApplicationInfo = &appInfo;

		if (layerNames.size() > 0)
		{
			instanceCreateInfo.enabledLayerCount = (uint32_t)layerNames.size();
			instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
		}

		instanceCreateInfo.enabledExtensionCount = (uint32_t)extNames.size();
		instanceCreateInfo.ppEnabledExtensionNames = extNames.data();



		error = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);

		Assert(error == VK_SUCCESS, "could not create instance of vulkan");

		if (layerNames.size() > 0 && error == VK_SUCCESS)
		{
			GET_VULKAN_FUNCTION_POINTER_INST(vkInstance, CreateDebugReportCallbackEXT);
			GET_VULKAN_FUNCTION_POINTER_INST(vkInstance, DestroyDebugReportCallbackEXT);
		}

		return vkInstance;
	}

	//fill the gpuCount param with the number of physical devices available to the program, and return a pointer to a vector containing the physical devices
	//returns a vector of the physical devices handles.
	std::vector<VkPhysicalDevice> EnumeratePhysicalDevices(VkInstance vkInstance, uint32_t* gpuCount)
	{

		VkResult error;
		error = vkEnumeratePhysicalDevices(vkInstance, gpuCount, nullptr);
		Assert(error, "could not enumerate gpus");
		Assert(*gpuCount > 0, "no compatible vulkan gpus available");

		std::vector<VkPhysicalDevice> physicalDevices(*gpuCount);
		error = vkEnumeratePhysicalDevices(vkInstance, gpuCount, physicalDevices.data());
		Assert(error, "could not enumerate physical devices (2nd usage of 2)");

		return physicalDevices;
	}


	VkDevice NewLogicalDevice(VkPhysicalDevice physicalDevice, uint32_t renderingQueueFamilyIndex, std::vector<const char*> deviceLayers, std::vector<const char*> deviceExts, VkPhysicalDeviceFeatures* features = nullptr)
	{

		float queuePriorities[1] = { 0.0f };
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = renderingQueueFamilyIndex;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = queuePriorities;


		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.pEnabledFeatures = features;
		deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExts.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExts.data();
		if (deviceLayers.size() > 0)
		{
			deviceCreateInfo.enabledLayerCount = (uint32_t)deviceLayers.size();
			deviceCreateInfo.ppEnabledLayerNames = deviceLayers.data();
		}

		VkResult error;
		VkDevice logicalDevice;
		error = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);

		Assert(error, "could not create logical device");
		return logicalDevice;
	}

	VkFormat GetSupportedDepthFormat(VkPhysicalDevice physicalDevice)
	{
		VkFormat depthFormats[] =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};
		VkFormat depthFormat = {};
		bool found = false;
		for (uint32_t i = 0; i < sizeof(depthFormats) / sizeof(VkFormat); i++)
		{
			VkFormatProperties formatProps;
			vkGetPhysicalDeviceFormatProperties(physicalDevice, depthFormats[i], &formatProps);
			if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				depthFormat = depthFormats[i];
				found = true;
				break;
			}

		}
		Assert(found == true, "could not find a supported depth format");
		return depthFormat;
	}

	VkSemaphore NewSemaphore(VkDevice logicalDevice)
	{
		VkSemaphoreCreateInfo semaphoreCreateInfo = {};
		semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		VkSemaphore semaphore;
		VkResult error = vkCreateSemaphore(logicalDevice, &semaphoreCreateInfo, nullptr, &semaphore);
		Assert(error, "could not create semaphore");
		return semaphore;
	}

	VkCommandPool NewCommandPool(VkDevice logicalDevice, uint32_t queueFamilyIndex)
	{
		VkResult error;
		VkCommandPool pool;
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		error = vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &pool);
		Assert(error, "could not create command pool.");
		return pool;
	}

	VkCommandBuffer NewSetupCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool)
	{
		VkCommandBuffer setupBuffer;
		VkResult error;
		VkCommandBufferAllocateInfo bufferAllocInfo = {};
		bufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAllocInfo.commandPool = cmdPool;
		bufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAllocInfo.commandBufferCount = 1;


		error = vkAllocateCommandBuffers(logicalDevice, &bufferAllocInfo, &setupBuffer);
		Assert(error, "could not create setup command buffer");
		VkCommandBufferBeginInfo setupBeginInfo = {};
		setupBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		error = vkBeginCommandBuffer(setupBuffer, &setupBeginInfo);
		Assert(error, "could not begin setup command buffer.");
		return setupBuffer;

	}

	void SetImageLayout(VkCommandBuffer cmdBuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageMemoryBarrier = {};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Undefined layout
		// Only allowed as initial layout!
		// Make sure any writes to the image have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		// Old layout is color attachment
		// Make sure any writes to the color buffer have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		// Old layout is depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		// Old layout is transfer source
		// Make sure any reads from the image have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// Old layout is shader read (sampler, input attachment)
		// Make sure any shader reads from the image have been finished
		if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

		// Target layouts (new)

		// New layout is transfer destination (copy, blit)
		// Make sure any copyies to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}

		// New layout is transfer source (copy, blit)
		// Make sure any reads from and writes to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// New layout is color attachment
		// Make sure any writes to the color buffer hav been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}

		// New layout is depth attachment
		// Make sure any writes to depth/stencil buffer have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}

		// New layout is shader read (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}

		// Put barrier on top
		VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags destStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		// Put barrier inside setup command buffer
		vkCmdPipelineBarrier(
			cmdBuffer,
			srcStageFlags,
			destStageFlags,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}



	std::vector<VkLayerProperties> GetInstalledVkLayers()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
		std::vector<VkLayerProperties> layerProps(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, layerProps.data());

		return layerProps;
	}

	std::vector<VkExtensionProperties> GetInstalledVkExtensions()
	{
		uint32_t layerCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &layerCount, nullptr);
		std::vector<VkExtensionProperties> extProps(layerCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &layerCount, extProps.data());
		return extProps;
	}

	std::vector<VkLayerProperties> GetInstalledVkLayers(VkPhysicalDevice physicalDevice)
	{
		uint32_t layerCount = 0;

		vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, nullptr);
		std::vector<VkLayerProperties> layerProps(layerCount);
		vkEnumerateDeviceLayerProperties(physicalDevice, &layerCount, layerProps.data());

		return layerProps;
	}

	std::vector<VkExtensionProperties> GetInstalledVkExtensions(VkPhysicalDevice physicalDevice)
	{
		uint32_t layerCount = 0;

		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &layerCount, nullptr);
		std::vector<VkExtensionProperties> extProps(layerCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &layerCount, extProps.data());

		return extProps;
	}


	VkCommandBuffer NewCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool)
	{
		VkCommandBuffer cmdBuffer;
		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = cmdPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;

		VkResult error = vkAllocateCommandBuffers(logicalDevice, &cmdAllocInfo, &cmdBuffer);
		Assert(error == VK_SUCCESS, "could not create command buffer");
		return cmdBuffer;
	}

	std::vector<VkCommandBuffer> NewCommandBuffer(VkDevice logicalDevice, VkCommandPool cmdPool, uint32_t numBuffers)
	{
		std::vector<VkCommandBuffer> cmdBuffers(numBuffers);
		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = cmdPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = numBuffers;

		VkResult error = vkAllocateCommandBuffers(logicalDevice, &cmdAllocInfo, cmdBuffers.data());
		Assert(error, "could not create command buffers");
		return cmdBuffers;
	}



	void GetMemoryType(VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t typeBits, VkFlags properties, uint32_t* typeIndex)
	{
		for (uint32_t i = 0; i < 32; i++)
		{
			if (((typeBits & 1) == 1) && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			{
				*typeIndex = i;
				return;
			}
			typeBits >>= 1;
		}

	}

	void setupDepthStencil(DeviceInfo* deviceInfo,
		const PhysDeviceInfo* physDeviceInfo,
		uint32_t width,
		uint32_t height)
	{
		VkImageCreateInfo image = {};
		image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		image.imageType = VK_IMAGE_TYPE_2D;
		image.format = physDeviceInfo->supportedDepthFormat;
		image.extent = { width, height, 1 };
		image.mipLevels = 1;
		image.arrayLayers = 1;
		image.samples = VK_SAMPLE_COUNT_1_BIT;
		image.tiling = VK_IMAGE_TILING_OPTIMAL;
		image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

		VkMemoryAllocateInfo mAlloc = {};
		mAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		VkImageViewCreateInfo ivInfo = {};
		ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivInfo.format = physDeviceInfo->supportedDepthFormat;
		ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		ivInfo.subresourceRange.baseMipLevel = 0;
		ivInfo.subresourceRange.levelCount = 1;
		ivInfo.subresourceRange.baseArrayLayer = 0;
		ivInfo.subresourceRange.layerCount = 1;

		VkMemoryRequirements memReqs;
		VkResult error;

		error = vkCreateImage(deviceInfo->device, &image, nullptr, &deviceInfo->depthStencil.image);
		Assert(error, "could not create vk image");
		vkGetImageMemoryRequirements(deviceInfo->device, deviceInfo->depthStencil.image, &memReqs);
		mAlloc.allocationSize = memReqs.size;
		GetMemoryType(physDeviceInfo->memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mAlloc.memoryTypeIndex);
		error = vkAllocateMemory(deviceInfo->device, &mAlloc, nullptr, &deviceInfo->depthStencil.mem);
		Assert(error, "could not allocate memory on device");

		error = vkBindImageMemory(deviceInfo->device, deviceInfo->depthStencil.image, deviceInfo->depthStencil.mem, 0);
		Assert(error, "could not bind image to memory");


		SetImageLayout(deviceInfo->setupCmdBuffer,
			deviceInfo->depthStencil.image,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		ivInfo.image = deviceInfo->depthStencil.image;
		error = vkCreateImageView(deviceInfo->device, &ivInfo, nullptr, &deviceInfo->depthStencil.view);
		Assert(error, "could not create image view");
	}

	VkRenderPass NewRenderPass(VkDevice logicalDevice, VkFormat surfaceColorFormat, VkFormat depthFormat)
	{
		VkAttachmentDescription attach[2] = {};
		attach[0].format = surfaceColorFormat;
		attach[0].samples = VK_SAMPLE_COUNT_1_BIT;
		attach[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attach[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attach[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attach[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attach[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attach[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		attach[1].format = depthFormat;
		attach[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attach[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attach[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attach[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attach[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attach[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attach[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference colorRef = {};
		colorRef.attachment = 0;
		colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depthRef = {};
		depthRef.attachment = 1;
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.flags = 0;
		subpass.inputAttachmentCount = 0;
		subpass.pInputAttachments = nullptr;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorRef;
		subpass.pResolveAttachments = nullptr;
		subpass.pDepthStencilAttachment = &depthRef;
		subpass.preserveAttachmentCount = 0;
		subpass.pPreserveAttachments = nullptr;

		VkRenderPassCreateInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpInfo.pNext = nullptr;
		rpInfo.attachmentCount = 2;
		rpInfo.pAttachments = attach;
		rpInfo.subpassCount = 1;
		rpInfo.pSubpasses = &subpass;
		rpInfo.dependencyCount = 0;
		rpInfo.pDependencies = nullptr;

		VkRenderPass rp;
		VkResult error = vkCreateRenderPass(logicalDevice, &rpInfo, nullptr, &rp);
		Assert(error, "could not create renderpass");
		return rp;
	}

	VkPipelineCache NewPipelineCache(VkDevice logicalDevice)
	{
		VkPipelineCache plCache;
		VkPipelineCacheCreateInfo plcInfo = {};
		plcInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VkResult error = vkCreatePipelineCache(logicalDevice, &plcInfo, nullptr, &plCache);
		Assert(error, "could not create pipeline cache");
		return plCache;
	}

	std::vector<VkFramebuffer> NewFrameBuffer(VkDevice logicalDevice,
		std::vector<VkImageView>* surfaceViews,
		VkRenderPass renderPass,
		VkImageView depthStencilView,
		uint32_t numBuffers,
		uint32_t width,
		uint32_t height)
	{
		std::vector<VkImageView> attach(surfaceViews->size());
		attach[1] = depthStencilView;
		VkFramebufferCreateInfo fbInfo = {};
		fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		fbInfo.renderPass = renderPass;
		fbInfo.attachmentCount = 2;
		fbInfo.pAttachments = attach.data();
		fbInfo.width = width;
		fbInfo.height = height;
		fbInfo.layers = 1;

		VkResult error;
		std::vector<VkFramebuffer> frameBuffers(numBuffers);
		for (uint32_t i = 0; i < numBuffers; i++)
		{
			attach[0] = surfaceViews->at(i);
			error = vkCreateFramebuffer(logicalDevice, &fbInfo, nullptr, &frameBuffers[i]);
			Assert(error, "could not create frame buffer");

		}
		return frameBuffers;
	}

	void FlushSetupCommandBuffer(DeviceInfo* deviceInfo)
	{
		VkResult error;
		if (deviceInfo->setupCmdBuffer == nullptr)
			return;
		error = vkEndCommandBuffer(deviceInfo->setupCmdBuffer);
		Assert(error, "could not end setup command buffer");

		VkSubmitInfo sInfo = {};
		sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sInfo.commandBufferCount = 1;
		sInfo.pCommandBuffers = &deviceInfo->setupCmdBuffer;

		error = vkQueueSubmit(deviceInfo->queue, 1, &sInfo, nullptr);
		Assert(error, "could not submit queue to setup cmd buffer");

		error = vkQueueWaitIdle(deviceInfo->queue);
		Assert(error, "wait idle failed for setupcmdbuffer");

		vkFreeCommandBuffers(deviceInfo->device, deviceInfo->cmdPool, 1, &deviceInfo->setupCmdBuffer);
		// ReSharper disable once CppAssignedValueIsNeverUsed
		deviceInfo->setupCmdBuffer = nullptr;
	}

	VkBuffer NewBuffer(VkDevice logicalDevice, uint32_t bufferSize, VkBufferUsageFlags usageBits)
	{
		VkBuffer buffer;
		VkBufferCreateInfo bInfo = {};
		bInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bInfo.size = bufferSize;
		bInfo.usage = usageBits;
		VkResult error = vkCreateBuffer(logicalDevice, &bInfo, nullptr, &buffer);
		Assert(error == VK_SUCCESS, "could not create buffer");
		return buffer;

	}


	StagingBuffer AllocBindDataToGPU(VkDevice logicalDevice, VkPhysicalDeviceMemoryProperties memoryProperties, uint32_t dataSize, void* dataTobind, VkBuffer* buffer, VkDeviceMemory* memory)
	{
		VkMemoryRequirements memReqs; //no stype
		VkMemoryAllocateInfo maInfo = {};
		maInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		StagingBuffer stagingBuffer;
		void* destPointer;

		stagingBuffer.buffer = NewBuffer(logicalDevice, dataSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		vkGetBufferMemoryRequirements(logicalDevice, stagingBuffer.buffer, &memReqs);
		maInfo.allocationSize = memReqs.size;
		GetMemoryType(memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &maInfo.memoryTypeIndex);
		VkResult error = vkAllocateMemory(logicalDevice, &maInfo, nullptr, &stagingBuffer.memory);
		Assert(error, "could not allocate memory in allocbinddatatogpu");
		//map and copy
		error = vkMapMemory(logicalDevice, stagingBuffer.memory, 0, maInfo.allocationSize, 0, &destPointer);
		Assert(error, "could not map memory in allocbinddatatogpu");
		memcpy(destPointer, dataTobind, dataSize);
		vkUnmapMemory(logicalDevice, stagingBuffer.memory);
		error = vkBindBufferMemory(logicalDevice, stagingBuffer.buffer, stagingBuffer.memory, 0);
		Assert(error, "could not bind memory in allocbinddatatogpu");

		vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
		maInfo.allocationSize = memReqs.size;
		GetMemoryType(memoryProperties, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &maInfo.memoryTypeIndex);
		error = vkAllocateMemory(logicalDevice, &maInfo, nullptr, memory);
		Assert(error, "could not allocate memory in allocbinddatatogpu");
		error = vkBindBufferMemory(logicalDevice, *buffer, *memory, 0);
		Assert(error, "could not bind memory in allocbinddatatogpu");

		return stagingBuffer;
	}

	VkMemoryAllocateInfo NewMemoryAllocInfo(VkDevice logicalDevice, VkPhysicalDeviceMemoryProperties memoryProperties, VkBuffer buffer, uint32_t desiredAllocSize, VkFlags desiredMemoryProperties)
	{
		VkMemoryAllocateInfo aInfo = {};
		aInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		aInfo.allocationSize = desiredAllocSize;
		VkMemoryRequirements mReqs;

		vkGetBufferMemoryRequirements(logicalDevice, buffer, &mReqs);
		aInfo.allocationSize = mReqs.size;
		GetMemoryType(memoryProperties, mReqs.memoryTypeBits, desiredMemoryProperties, &aInfo.memoryTypeIndex);

		return aInfo;
	}

	VkShaderModule NewShaderModule(VkDevice logicalDevice, uint32_t* data, uint32_t dataSize)
	{
		VkShaderModule shaderModule;
		VkShaderModuleCreateInfo mInfo = {};
		VkResult error;

		mInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		mInfo.codeSize = dataSize;
		mInfo.pCode = data;
		mInfo.flags = 0;

		error = vkCreateShaderModule(logicalDevice, &mInfo, nullptr, &shaderModule);
		Assert(error, "could not create shader module");
		return shaderModule;
	}

	VkPipelineShaderStageCreateInfo NewShaderStageInfo(VkDevice logicalDevice, PipelineInfo* pipelineInfo, uint32_t* data, uint32_t dataSize, VkShaderStageFlagBits stage)
	{
		VkShaderModule module = NewShaderModule(logicalDevice, data, dataSize);
		pipelineInfo->shaderModules.push_back(module);
		VkPipelineShaderStageCreateInfo sInfo = {};
		sInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		sInfo.stage = stage;
		sInfo.module = module;
		sInfo.pName = "main";
		return sInfo;
	}

	void DestroyPipelineInfo(VkDevice device, PipelineInfo* pipelineInfo)
	{
		vkDestroyDescriptorPool(device, pipelineInfo->descriptorPool, nullptr);
		vkDestroyPipeline(device, pipelineInfo->pipeline, nullptr);
		for (auto& shaderModule : pipelineInfo->shaderModules)
		{
			vkDestroyShaderModule(device, shaderModule, nullptr);
		}
		vkDestroyPipelineLayout(device, pipelineInfo->pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, pipelineInfo->descriptorSetLayout, nullptr);
		vkDestroyPipelineCache(device, pipelineInfo->pipelineCache, nullptr);
		vkDestroyRenderPass(device, pipelineInfo->renderPass, nullptr);

	}

	void DestroyDeviceInfo(DeviceInfo* deviceInfo)
	{

		vkFreeCommandBuffers(deviceInfo->device, deviceInfo->cmdPool, (uint32_t)deviceInfo->drawCmdBuffers.size(), deviceInfo->drawCmdBuffers.data());
		vkFreeCommandBuffers(deviceInfo->device, deviceInfo->cmdPool, 1, &deviceInfo->prePresentCmdBuffer);
		vkFreeCommandBuffers(deviceInfo->device, deviceInfo->cmdPool, 1, &deviceInfo->postPresentCmdBuffer);
		vkFreeCommandBuffers(deviceInfo->device, deviceInfo->cmdPool, 1, &deviceInfo->setupCmdBuffer);

		for (uint32_t i = 0; i < deviceInfo->frameBuffers.size(); i++)
		{
			vkDestroyFramebuffer(deviceInfo->device, deviceInfo->frameBuffers[i], nullptr);
		}

		vkDestroyImageView(deviceInfo->device, deviceInfo->depthStencil.view, nullptr);
		vkDestroyImage(deviceInfo->device, deviceInfo->depthStencil.image, nullptr);
		vkFreeMemory(deviceInfo->device, deviceInfo->depthStencil.mem, nullptr);

		vkDestroyCommandPool(deviceInfo->device, deviceInfo->cmdPool, nullptr);


		vkDestroySemaphore(deviceInfo->device, deviceInfo->presentComplete, nullptr);
		vkDestroySemaphore(deviceInfo->device, deviceInfo->renderComplete, nullptr);

		vkDestroyDevice(deviceInfo->device, nullptr);
	}

	void DestroyDebugInfo(VkInstance vkInstance, DebugInfo* debugInfo)
	{
		DestroyDebugReportCallbackEXT(vkInstance, debugInfo->debugReport, nullptr);
	}

	void DestroyInstance(VkInstance vkInstance)
	{
		vkDestroyInstance(vkInstance, nullptr);
	}
}
