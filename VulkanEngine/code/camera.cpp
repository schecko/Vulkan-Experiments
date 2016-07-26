
#include "cymath.h"
#include <vulkan.h>

#include "camera.h"
#include "util.h"
#include "commonvulkan.h"

namespace Cy
{

	void UpdateCamera(VkDevice logicalDevice, Camera& camera, uint32_t width, uint32_t height)
	{
		//check zoom
		if (camera.cameraPos.zoom > ZOOM_MAX)
		{
			camera.cameraPos.zoom = ZOOM_MAX;
		}
		if (camera.cameraPos.zoom < ZOOM_MIN)
		{
			camera.cameraPos.zoom = ZOOM_MIN;
		}

		//check pitch
		if (camera.cameraPos.pitch > PI / 2)
		{
			camera.cameraPos.pitch = PI / 2;
		}
		if (camera.cameraPos.pitch < -PI / 2)
		{
			camera.cameraPos.pitch = -PI / 2;
		}

		v3 tempFront;
		tempFront.x = cos(camera.cameraPos.yaw) * cos(camera.cameraPos.pitch);
		tempFront.y = sin(camera.cameraPos.pitch);
		tempFront.z = sin(camera.cameraPos.yaw) * cos(camera.cameraPos.pitch);
		camera.cameraPos.front = Norm(tempFront);
		camera.cameraPos.right = Norm(Cross(camera.cameraPos.front, camera.cameraPos.worldUp));
		camera.cameraPos.up = Norm(Cross(camera.cameraPos.right, camera.cameraPos.front));

		camera.cameraMats.view = LookAt(camera.cameraPos.position, camera.cameraPos.position + camera.cameraPos.front, camera.cameraPos.up);
		camera.cameraMats.projection = Perspective(camera.cameraPos.zoom, (float)width / (float)height, .1f, 256.0f);
		camera.cameraMats.model = Translate(m4(), v3(0.0f, 0.0f, 0.0f));

		void* destPointer;
		VkResult error = vkMapMemory(logicalDevice, camera.memory, 0, sizeof(CameraMats), 0, &destPointer);
		Assert(error, "could not map uniform data in update camera");
		memcpy(destPointer, &camera.cameraMats, sizeof(CameraMats));
		vkUnmapMemory(logicalDevice, camera.memory);
	}


	void PrepareCameraBuffers(VkDevice logicalDevice,
		VkPhysicalDeviceMemoryProperties memoryProperties,
		Camera* camera,
		uint32_t width,
		uint32_t height,
		float zoom,
		v3 rotation)
	{
		camera->buffer = NewBuffer(logicalDevice, sizeof(CameraMats), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		VkMemoryAllocateInfo aInfo = NewMemoryAllocInfo(logicalDevice, memoryProperties, camera->buffer, 0, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		VkResult error;
		error = vkAllocateMemory(logicalDevice, &aInfo, nullptr, &camera->memory);
		Assert(error, "could not allocate memory in prepapre camera buffers");
		error = vkBindBufferMemory(logicalDevice, camera->buffer, camera->memory, 0);
		Assert(error, "could not bind buffer memory in prepare camera buffers");

		camera->desc.buffer = camera->buffer;
		camera->desc.offset = 0;
		camera->desc.range = sizeof(CameraMats);
		UpdateCamera(logicalDevice, *camera, width, height);
	}

	Camera NewCamera(v3 initalPosition)
	{
		Camera camera = {};
		camera.cameraPos.position = initalPosition;
		camera.cameraPos.worldUp = v3(0.0f, 1.0f, 0.0f);
		camera.cameraPos.yaw = -PI / 2;
		camera.cameraPos.pitch = 0.0f;
		camera.cameraPos.zoom = PI / 6;

		return camera;
	}
}