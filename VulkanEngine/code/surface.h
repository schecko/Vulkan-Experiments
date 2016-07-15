#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include "commonvulkan.h"
#include "commonwindows.h"

namespace Cy
{
	struct WindowInfo;
	//todo remove cyclic dependance
	struct DeviceInfo;

	struct SurfaceInfo
	{
		VkSurfaceKHR surface;
		uint32_t renderingQueueFamilyIndex;
		VkColorSpaceKHR colorSpace;
		VkFormat colorFormat;

		//function pointers
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
	};

	struct SwapchainInfo
	{
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		VkSwapchainKHR swapChain;
		uint32_t currentBuffer;

		//function pointers
		PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
		PFN_vkDestroySwapchainKHR DestroySwapchainKHR;
		PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
		PFN_vkAcquireNextImageKHR AcquireNextImageKHR;
		PFN_vkQueuePresentKHR QueuePresentKHR;
	};

	VkSurfaceKHR NewSurface(const WindowInfo* window, VkInstance vkInstance);

	uint32_t FindGraphicsQueueFamilyIndex(VkPhysicalDevice vkPhysicalDevice, VkSurfaceKHR surface);

	void GetSurfaceColorSpaceAndFormat(VkPhysicalDevice physicalDevice,
		SurfaceInfo* surfaceInfo);

	void NewSwapchainInfo(WindowInfo* windowInfo,
		const PhysDeviceInfo* physDeviceInfo,
		const SurfaceInfo* surfaceInfo,
		const DeviceInfo* deviceInfo,
		SwapchainInfo* swapchainInfo);

	VkResult AcquireNextImage(const DeviceInfo* deviceInfo, SurfaceInfo* surfaceInfo);

	VkResult QueuePresent(const DeviceInfo* deviceInfo, const SurfaceInfo* surfaceInfo);

	void DestroySurfaceInfo(VkInstance vkInstance, VkDevice device, SurfaceInfo* surfaceInfo);

	void DestroySwapchainInfo(const DeviceInfo* deviceInfo, SwapchainInfo* swapchainInfo);
}