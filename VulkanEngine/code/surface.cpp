

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>
#include "util.h"
#include <vector>
#include "commonwindows.h"
#include "surface.h"

namespace Cy
{
	//function pointers
	//PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR = nullptr;
	//PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
	//PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
	//PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
	//PFN_vkCreateSwapchainKHR CreateSwapchainKHR = nullptr;
	//PFN_vkDestroySwapchainKHR DestroySwapchainKHR = nullptr;
	//PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR = nullptr;
	//PFN_vkAcquireNextImageKHR AcquireNextImageKHR = nullptr;
	//PFN_vkQueuePresentKHR QueuePresentKHR = nullptr;

#define GET_VULKAN_FUNCTION_POINTER_DEV(dev, function)								\
{																					\
	function = (PFN_vk##function) vkGetDeviceProcAddr(dev, "vk"#function);			\
	Assert(function != nullptr, "could not find function "#function);				\
}																					\



	VkSurfaceKHR NewSurface(VkInstance vkInstance, const WindowInfo* window)
	{
		VkResult error;
		VkSurfaceKHR surface;
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = window->exeHandle;
		surfaceCreateInfo.hwnd = window->windowHandle;
		error = vkCreateWin32SurfaceKHR(vkInstance,
			&surfaceCreateInfo,
			nullptr,
			&surface);

		Assert(error, "could not create windows surface");
		return surface;
	}

	//from the physicaldevice as a param, returns a grphics queueFamily cabable of a rendering pipeline.
	uint32_t FindGraphicsQueueFamilyIndex(VkPhysicalDevice vkPhysicalDevice, const SurfaceInfo* surfaceInfo)
	{

		uint32_t queueCount = 0;
		uint32_t graphicsAndPresentingQueueIndex = 0; //cannot be within for-loop scope, used a bit later
		VkBool32 supportsPresent = VK_FALSE;
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueCount, nullptr);
		Assert(queueCount > 0, "physical device has no available queues");

		std::vector<VkQueueFamilyProperties> queueProperties(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueCount, queueProperties.data());

		for (; graphicsAndPresentingQueueIndex < queueCount; graphicsAndPresentingQueueIndex++)
		{
			surfaceInfo->GetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, graphicsAndPresentingQueueIndex, surfaceInfo->surface, &supportsPresent);
			if (queueProperties[graphicsAndPresentingQueueIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT && supportsPresent)
				break;
		}

		Assert(graphicsAndPresentingQueueIndex < queueCount, "this gpu doesn't support graphics rendering.... what a useful gpu");

		return graphicsAndPresentingQueueIndex;
	}

	void GetSurfaceColorSpaceAndFormat(VkPhysicalDevice physicalDevice,
		SurfaceInfo* surfaceInfo)
	{
		uint32_t surfaceFormatCount;
		VkResult error;
		error = surfaceInfo->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surfaceInfo->surface, &surfaceFormatCount, nullptr);
		Assert(error, "could not get surface format counts, GetphysicalDeviceSurfaceFormatsKHR is probably null");
		Assert(surfaceFormatCount > 0, "surfaceformatcount is less than 1");

		std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
		error = surfaceInfo->GetPhysicalDeviceSurfaceFormatsKHR(physicalDevice,
			surfaceInfo->surface,
			&surfaceFormatCount,
			surfaceFormats.data());
		Assert(error, "could not get surface format counts, GetphysicalDeviceSurfaceFormatsKHR is probably null");

		if (surfaceFormatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			surfaceInfo->colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
		}
		else
		{
			surfaceInfo->colorFormat = surfaceFormats[0].format;
		}
		surfaceInfo->colorSpace = surfaceFormats[0].colorSpace;
	}

	static void SetImageLayout(VkCommandBuffer cmdBuffer,
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


	static VkSurfaceCapabilitiesKHR GetSurfaceCaps(VkPhysicalDevice physDevice, const SurfaceInfo* surfaceInfo)
	{
		VkSurfaceCapabilitiesKHR surfaceCaps = {};
		VkResult error;
		error = surfaceInfo->GetPhysicalDeviceSurfaceCapabilitiesKHR(physDevice,
			surfaceInfo->surface,
			&surfaceCaps);
		Assert(error, "could not get surface capabilities, the function is probably null?");
		return surfaceCaps;

	}

	static std::vector<VkPresentModeKHR> GetPresentModes(VkPhysicalDevice physDevice, const SurfaceInfo* surfaceInfo)
	{

		uint32_t presentModeCount;
		VkResult error;

		error = surfaceInfo->GetPhysicalDeviceSurfacePresentModesKHR(physDevice,
			surfaceInfo->surface,
			&presentModeCount,
			nullptr);
		Assert(error, "could not get surface present modes");
		Assert(presentModeCount > 0, "present mode count is zero!!");

		std::vector<VkPresentModeKHR> presentModes(presentModeCount);


		error = surfaceInfo->GetPhysicalDeviceSurfacePresentModesKHR(physDevice,
			surfaceInfo->surface,
			&presentModeCount,
			presentModes.data());
		Assert(error, "could not get the present Modes");
		return presentModes;

	}

	VkPresentModeKHR GetPresentMode(std::vector<VkPresentModeKHR>* presentModes)
	{
		VkPresentModeKHR swapChainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
		for (size_t i = 0; i < presentModes->size(); i++)
		{
			if (presentModes->at(i) == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				swapChainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
				break;
			}
			if (swapChainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR &&
				presentModes->at(i) == VK_PRESENT_MODE_IMMEDIATE_KHR)
			{
				swapChainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
		return swapChainPresentMode;
	}

	VkExtent2D GetSurfaceExtent(VkSurfaceCapabilitiesKHR* surfaceCaps, uint32_t* clientWidth, uint32_t* clientHeight)
	{
		VkExtent2D surfaceExtant = {};
		//width and height are either both -1, or both not -1
		if (surfaceCaps->currentExtent.width == -1)
		{
			surfaceExtant.width = *clientWidth;
			surfaceExtant.height = *clientHeight;
		}
		else
		{
			surfaceExtant = surfaceCaps->currentExtent;
			*clientWidth = surfaceCaps->currentExtent.width;
			*clientHeight = surfaceCaps->currentExtent.height;
		}
		return surfaceExtant;
	}

	uint32_t GetDesiredNumSwapChainImages(const VkSurfaceCapabilitiesKHR* surfaceCaps)
	{
		//determine the number of images
		uint32_t desiredNumberOfSwapChainImages = surfaceCaps->minImageCount + 1;
		if (surfaceCaps->maxImageCount > 0 && desiredNumberOfSwapChainImages > surfaceCaps->maxImageCount)
		{
			desiredNumberOfSwapChainImages = surfaceCaps->maxImageCount;
		}
		return desiredNumberOfSwapChainImages;
	}

	VkSurfaceTransformFlagBitsKHR GetSurfaceTransformBits(const VkSurfaceCapabilitiesKHR* surfaceCaps)
	{
		VkSurfaceTransformFlagBitsKHR preTransform;
		if (surfaceCaps->supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfaceCaps->currentTransform;
		}
		return preTransform;
	}

	void NewSwapchainInfo(WindowInfo* windowInfo,
		const PhysDeviceInfo* physDeviceInfo,
		const SurfaceInfo* surfaceInfo,
		const DeviceInfo* deviceInfo,
		SwapchainInfo* swapchainInfo)
	{
		//TODO parts of this function could be moved to other functions to improve the flow of initialization?
		//specifically move the use of the function extensions into a single function?
		VkResult error;

		//after logical device creation we can retrieve function pointers associated with it
		GET_VULKAN_FUNCTION_POINTER_DEVSWAPCHAIN(deviceInfo->device, CreateSwapchainKHR);
		GET_VULKAN_FUNCTION_POINTER_DEVSWAPCHAIN(deviceInfo->device, DestroySwapchainKHR);
		GET_VULKAN_FUNCTION_POINTER_DEVSWAPCHAIN(deviceInfo->device, GetSwapchainImagesKHR);
		GET_VULKAN_FUNCTION_POINTER_DEVSWAPCHAIN(deviceInfo->device, AcquireNextImageKHR);
		GET_VULKAN_FUNCTION_POINTER_DEVSWAPCHAIN(deviceInfo->device, QueuePresentKHR);

		VkSurfaceCapabilitiesKHR surfaceCaps = GetSurfaceCaps(physDeviceInfo->physicalDevice, surfaceInfo);
		std::vector<VkPresentModeKHR> presentModes = GetPresentModes(physDeviceInfo->physicalDevice, surfaceInfo);

		VkSwapchainCreateInfoKHR scInfo = {};
		scInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		scInfo.surface = surfaceInfo->surface;
		scInfo.minImageCount = GetDesiredNumSwapChainImages(&surfaceCaps);
		scInfo.imageFormat = surfaceInfo->colorFormat;
		scInfo.imageColorSpace = surfaceInfo->colorSpace;
		scInfo.imageExtent = GetSurfaceExtent(&surfaceCaps, &windowInfo->clientWidth, &windowInfo->clientHeight);
		scInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		scInfo.preTransform = GetSurfaceTransformBits(&surfaceCaps);
		scInfo.imageArrayLayers = 1;
		scInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		scInfo.presentMode = GetPresentMode(&presentModes);
		//TODO do I ever have an old swapchain and need to create a new one?
		//swapchainCI.oldSwapchain = oldSwapChain;
		scInfo.clipped = VK_TRUE;
		scInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		error = swapchainInfo->CreateSwapchainKHR(deviceInfo->device, &scInfo, nullptr, &swapchainInfo->swapChain);
		Assert(error, "could not create a swapchain");

		error = swapchainInfo->GetSwapchainImagesKHR(deviceInfo->device, swapchainInfo->swapChain, &swapchainInfo->imageCount, nullptr);
		Assert(error, "could not get surface image count");
		swapchainInfo->images.resize(swapchainInfo->imageCount);
		swapchainInfo->views.resize(swapchainInfo->imageCount);
		error = swapchainInfo->GetSwapchainImagesKHR(deviceInfo->device, swapchainInfo->swapChain, &swapchainInfo->imageCount, swapchainInfo->images.data());
		Assert(error, "could not fill surface images vector");

		for (uint32_t i = 0; i < swapchainInfo->imageCount; i++)
		{
			VkImageViewCreateInfo ivInfo = {};
			ivInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			ivInfo.format = surfaceInfo->colorFormat;
			ivInfo.components = {
				VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A
			};
			ivInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			ivInfo.subresourceRange.baseMipLevel = 0;
			ivInfo.subresourceRange.levelCount = 1;
			ivInfo.subresourceRange.baseArrayLayer = 0;
			ivInfo.subresourceRange.layerCount = 1;
			ivInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			ivInfo.flags = 0;

			SetImageLayout(deviceInfo->setupCmdBuffer,
				swapchainInfo->images[i],
				VK_IMAGE_ASPECT_COLOR_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

			ivInfo.image = swapchainInfo->images[i];
			error = vkCreateImageView(deviceInfo->device, &ivInfo, nullptr, &swapchainInfo->views[i]);
			Assert(error, "could not create image view");
		}
	}

	VkResult AcquireNextImage(const DeviceInfo* deviceInfo, SwapchainInfo* swapchainInfo)
	{
		return swapchainInfo->AcquireNextImageKHR(deviceInfo->device, swapchainInfo->swapChain, UINT64_MAX, deviceInfo->presentComplete, nullptr, &swapchainInfo->currentBuffer);
	}

	VkResult QueuePresent(const DeviceInfo* deviceInfo, const SwapchainInfo* swapchainInfo)
	{
		VkPresentInfoKHR pInfo = {};
		pInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		pInfo.swapchainCount = 1;
		pInfo.pSwapchains = &swapchainInfo->swapChain;
		pInfo.pImageIndices = &swapchainInfo->currentBuffer;
		pInfo.waitSemaphoreCount = 1;
		pInfo.pWaitSemaphores = &deviceInfo->renderComplete;
		return swapchainInfo->QueuePresentKHR(deviceInfo->queue, &pInfo);
	}

	void NewSurfaceInfo(const WindowInfo* windowInfo, const InstanceInfo* instanceInfo, const PhysDeviceInfo* physDeviceInfo, SurfaceInfo* surfaceInfo)
	{

		surfaceInfo->surface = NewSurface(instanceInfo->vkInstance, windowInfo);

		//get function pointers after creating instance of vulkan
		GET_VULKAN_FUNCTION_POINTER_INSTSURFACE(instanceInfo->vkInstance, GetPhysicalDeviceSurfaceSupportKHR);
		GET_VULKAN_FUNCTION_POINTER_INSTSURFACE(instanceInfo->vkInstance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
		GET_VULKAN_FUNCTION_POINTER_INSTSURFACE(instanceInfo->vkInstance, GetPhysicalDeviceSurfaceFormatsKHR);
		GET_VULKAN_FUNCTION_POINTER_INSTSURFACE(instanceInfo->vkInstance, GetPhysicalDeviceSurfacePresentModesKHR);
		surfaceInfo->renderingQueueFamilyIndex = FindGraphicsQueueFamilyIndex(physDeviceInfo->physicalDevice, surfaceInfo);
		GetSurfaceColorSpaceAndFormat(physDeviceInfo->physicalDevice, surfaceInfo);
	}


	void DestroySurfaceInfo(const InstanceInfo* instanceInfo, SurfaceInfo* surfaceInfo)
	{

		vkDestroySurfaceKHR(instanceInfo->vkInstance, surfaceInfo->surface, nullptr);

		surfaceInfo->GetPhysicalDeviceSurfaceSupportKHR = nullptr;
		surfaceInfo->GetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
		surfaceInfo->GetPhysicalDeviceSurfaceFormatsKHR = nullptr;
		surfaceInfo->GetPhysicalDeviceSurfacePresentModesKHR = nullptr;
	}

	void DestroySwapchainInfo(const DeviceInfo* deviceInfo, SwapchainInfo* swapchainInfo)
	{
		for (uint32_t i = 0; i < swapchainInfo->imageCount; i++)
		{
			//TODO is image destroyed along with its associated imageview?
			//vkDestroyImage(device, surfaceInfo->images[i], nullptr);
			vkDestroyImageView(deviceInfo->device, swapchainInfo->views[i], nullptr);

		}

		swapchainInfo->DestroySwapchainKHR(deviceInfo->device, swapchainInfo->swapChain, nullptr);

		swapchainInfo->CreateSwapchainKHR = nullptr;
		swapchainInfo->DestroySwapchainKHR = nullptr;
		swapchainInfo->GetSwapchainImagesKHR = nullptr;
		swapchainInfo->AcquireNextImageKHR = nullptr;
		swapchainInfo->QueuePresentKHR = nullptr;
	}
}
