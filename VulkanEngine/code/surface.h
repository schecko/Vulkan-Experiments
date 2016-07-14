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
		VkColorSpaceKHR colorSpace;
		VkFormat colorFormat;
		uint32_t imageCount;
		std::vector<VkImage> images;
		std::vector<VkImageView> views;
		VkSwapchainKHR swapChain;
		uint32_t currentBuffer;

		//function pointers
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR GetPhysicalDeviceSurfaceSupportKHR;
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR GetPhysicalDeviceSurfaceCapabilitiesKHR;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR GetPhysicalDeviceSurfaceFormatsKHR;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR GetPhysicalDeviceSurfacePresentModesKHR;
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

	void InitSwapChain(
		const DeviceInfo* deviceInfo,
		VkPhysicalDevice physDevice,
		SurfaceInfo* surfaceInfo,
		uint32_t* width,
		uint32_t* height);

	VkResult AcquireNextImage(const DeviceInfo* deviceInfo, SurfaceInfo* surfaceInfo);

	VkResult QueuePresent(const DeviceInfo* deviceInfo, const SurfaceInfo* surfaceInfo);

	void DestroySurfaceInfo(VkInstance vkInstance, VkDevice device, SurfaceInfo* surfaceInfo);
}