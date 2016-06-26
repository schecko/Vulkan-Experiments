#pragma once

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>


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
		glm::mat4 projection;
		glm::mat4 model;
		glm::mat4 view;

	};

	struct CameraPos
	{
		glm::vec3 position;
		glm::vec3 worldUp;

		glm::vec3 up;
		glm::vec3 front;
		glm::vec3 right;

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
		glm::vec3 rotation);

	void UpdateCamera(VkDevice logicalDevice, Camera& camera, uint32_t width, uint32_t height);

	CameraPos NewCameraPos(glm::vec3 initalPosition = glm::vec3(0.0f, 0.0f, 10.0f));
}