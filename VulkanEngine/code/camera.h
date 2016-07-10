#pragma once

#define GLM_FORCE_RADIANS
#include "cymath.h"
#include <vulkan.h>


namespace Cy
{
	const float PI = 3.1415926535897932384626433832795028841971f; //overkill but i like overkill
	const float ZOOM_MAX = PI / 6;
	const float ZOOM_MIN = PI / 3;
	const float ZOOM_SPEED = .2f;
	const float ANGLE_SPEED = PI / 12;

	//do not add anything to this struct, since its used in shader calculations as uniforms
	struct CameraMats
	{
		m4 projection;
		m4 model;
		m4 view;

	};

	struct CameraPos
	{
		v3 position;
		v3 worldUp;

		v3 up;
		v3 front;
		v3 right;

		float yaw;
		float pitch;
		float zoom;
	};

	struct Camera
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		struct VkDescriptorBufferInfo desc;

		CameraMats cameraMats;
		CameraPos cameraPos;
	};

	void PrepareCameraBuffers(VkDevice logicalDevice,
		VkPhysicalDeviceMemoryProperties memoryProperties,
		Camera* camera,
		uint32_t width,
		uint32_t height,
		float zoom,
		v3 rotation);

	void UpdateCamera(VkDevice logicalDevice, Camera& camera, uint32_t width, uint32_t height);

	CameraPos NewCameraPos(v3 initalPosition = v3(0.0f, 0.0f, 10.0f));
}
