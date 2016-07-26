

#define VK_USE_PLATFORM_WIN32_KHR
#include "win64_vulkantriangle.h"
#include <vulkan.h>


#include <windows.h>
#include <vector>
#include "commonvulkan.h"
#include "commonwindows.h"
#include "surface.h"
#include "util.h"
#include "camera.h"

using namespace Cy;


	void PrepareVertexData(const DeviceInfo* deviceInfo, VkPhysicalDeviceMemoryProperties memoryProperties, VertexBuffer* vertexBuffer)
	{
		size_t vertexBufferSize = vertexBuffer->vPos.size() * sizeof(Vertex);
		size_t indexBufferSize = vertexBuffer->iPos.size() * sizeof(uint32_t);

		StagingBuffers stagingBuffers;

		VkCommandBuffer copyCommandBuffer = NewCommandBuffer(deviceInfo->device, deviceInfo->cmdPool);
		vertexBuffer->vBuffer = NewBuffer(deviceInfo->device, vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		vertexBuffer->iBuffer = NewBuffer(deviceInfo->device, indexBufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

		stagingBuffers.vertices = AllocBindDataToGPU(deviceInfo->device, memoryProperties, vertexBufferSize, vertexBuffer->vPos.data(), &vertexBuffer->vBuffer, &vertexBuffer->vMemory);
		stagingBuffers.indices = AllocBindDataToGPU(deviceInfo->device, memoryProperties, indexBufferSize, vertexBuffer->iPos.data(), &vertexBuffer->iBuffer, &vertexBuffer->iMemory);

		VkCommandBufferBeginInfo cbbInfo = {};
		cbbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkBufferCopy copyRegion = {};
		VkResult error = vkBeginCommandBuffer(copyCommandBuffer, &cbbInfo);
		Assert(error, "could not begin copy cmd buffer for vertex data");
		copyRegion.size = vertexBufferSize;

		vkCmdCopyBuffer(copyCommandBuffer, stagingBuffers.vertices.buffer, vertexBuffer->vBuffer, 1, &copyRegion);
		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(copyCommandBuffer, stagingBuffers.indices.buffer, vertexBuffer->iBuffer, 1, &copyRegion);

		error = vkEndCommandBuffer(copyCommandBuffer);
		Assert(error, "could not end copy cmd buffer for vertex data");

		//submit copies to queue
		VkSubmitInfo csInfo = {};
		csInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		csInfo.commandBufferCount = 1;
		csInfo.pCommandBuffers = &copyCommandBuffer;

		error = vkQueueSubmit(deviceInfo->queue, 1, &csInfo, nullptr);
		Assert(error, "could not submit cmd buffer to copy data");
		error = vkQueueWaitIdle(deviceInfo->queue);
		Assert(error, "wait for queue during vertex data copy failed");

		//TODO sync?
		vkDestroyBuffer(deviceInfo->device, stagingBuffers.vertices.buffer, nullptr);
		vkFreeMemory(deviceInfo->device, stagingBuffers.vertices.memory, nullptr);
		vkDestroyBuffer(deviceInfo->device, stagingBuffers.indices.buffer, nullptr);
		vkFreeMemory(deviceInfo->device, stagingBuffers.indices.memory, nullptr);

		//binding desc
		vertexBuffer->vBindingDesc.resize(1);
		vertexBuffer->vBindingDesc[0].binding = VERTEX_BUFFER_BIND_ID;
		vertexBuffer->vBindingDesc[0].stride = sizeof(Vertex);
		vertexBuffer->vBindingDesc[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		vertexBuffer->vBindingAttr.resize(2);
		//location 0 is cameraMats
		vertexBuffer->vBindingAttr[0].binding = VERTEX_BUFFER_BIND_ID;
		vertexBuffer->vBindingAttr[0].location = 0;
		vertexBuffer->vBindingAttr[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexBuffer->vBindingAttr[0].offset = 0;
		vertexBuffer->vBindingAttr[0].binding = 0;

		vertexBuffer->vBindingAttr[1].binding = VERTEX_BUFFER_BIND_ID;
		vertexBuffer->vBindingAttr[1].location = 1;
		vertexBuffer->vBindingAttr[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		vertexBuffer->vBindingAttr[1].offset = sizeof(float) * 3;
		vertexBuffer->vBindingAttr[1].binding = 0;

		vertexBuffer->viInfo = {}; //must zero init to ensure pnext is zero (and everything else isnt trash)
		vertexBuffer->viInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexBuffer->viInfo.vertexBindingDescriptionCount = vertexBuffer->vBindingDesc.size();
		vertexBuffer->viInfo.pVertexBindingDescriptions = vertexBuffer->vBindingDesc.data();
		vertexBuffer->viInfo.vertexAttributeDescriptionCount = vertexBuffer->vBindingAttr.size();
		vertexBuffer->viInfo.pVertexAttributeDescriptions = vertexBuffer->vBindingAttr.data();

	}

	void SetupCommandBuffers(DeviceInfo* deviceInfo,
		uint32_t swapchainImageCount)
	{
		deviceInfo->drawCmdBuffers.resize(swapchainImageCount);
		deviceInfo->drawCmdBuffers = NewCommandBuffer(deviceInfo->device, deviceInfo->cmdPool, swapchainImageCount);
		deviceInfo->prePresentCmdBuffer = NewCommandBuffer(deviceInfo->device, deviceInfo->cmdPool);
		deviceInfo->postPresentCmdBuffer = NewCommandBuffer(deviceInfo->device, deviceInfo->cmdPool);
	}

	VkPipeline NewPipeline(VkDevice logicalDevice, PipelineInfo* pipelineInfo, VkPipelineVertexInputStateCreateInfo* vertexInputInfo)
	{

		VkResult error;

		//vertex input
		VkPipelineInputAssemblyStateCreateInfo iaInfo = {};
		iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		iaInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		//rasterization state
		VkPipelineRasterizationStateCreateInfo rInfo = {};
		rInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rInfo.polygonMode = VK_POLYGON_MODE_FILL;
		rInfo.cullMode = VK_CULL_MODE_NONE;
		rInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rInfo.depthClampEnable = VK_FALSE;
		rInfo.rasterizerDiscardEnable = VK_FALSE;
		rInfo.depthBiasEnable = VK_FALSE;

		//color blend state (only one here)
		VkPipelineColorBlendAttachmentState baState[1] = {};
		baState[0].colorWriteMask = 0xf;
		baState[0].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo cbInfo = {};
		cbInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cbInfo.attachmentCount = 1;
		cbInfo.pAttachments = baState;

		//viewport state
		VkPipelineViewportStateCreateInfo vInfo = {};
		vInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vInfo.viewportCount = 1;
		vInfo.scissorCount = 1;

		//dynamic states
		std::vector<VkDynamicState> dEnables;
		dEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
		dEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

		VkPipelineDynamicStateCreateInfo dInfo = {};
		dInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dInfo.dynamicStateCount = dEnables.size();
		dInfo.pDynamicStates = dEnables.data();

		//depth and stencil
		VkPipelineDepthStencilStateCreateInfo dsInfo = {};
		dsInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		dsInfo.depthTestEnable = VK_TRUE;
		dsInfo.depthWriteEnable = VK_TRUE;
		dsInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		dsInfo.depthBoundsTestEnable = VK_FALSE;
		dsInfo.back.failOp = VK_STENCIL_OP_KEEP;
		dsInfo.back.passOp = VK_STENCIL_OP_KEEP;
		dsInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
		dsInfo.stencilTestEnable = VK_FALSE;
		dsInfo.front = dsInfo.back;

		//multisampling
		VkPipelineMultisampleStateCreateInfo msInfo = {};
		msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		msInfo.pSampleMask = nullptr;
		msInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		File vertexShader = OpenFile("D:\\scottsdocs\\sourcecode\\VulkanTriangle\\code\\triangle.vert.spv");
		File fragmentShader = OpenFile("D:\\scottsdocs\\sourcecode\\VulkanTriangle\\code\\triangle.frag.spv");

		VkPipelineShaderStageCreateInfo sInfo[2] = {};
		sInfo[0] = NewShaderStageInfo(logicalDevice, pipelineInfo, (uint32_t*)vertexShader.data, vertexShader.size, VK_SHADER_STAGE_VERTEX_BIT);
		sInfo[1] = NewShaderStageInfo(logicalDevice, pipelineInfo, (uint32_t*)fragmentShader.data, fragmentShader.size, VK_SHADER_STAGE_FRAGMENT_BIT);

		VkGraphicsPipelineCreateInfo pInfo = {};
		pInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pInfo.layout = pipelineInfo->pipelineLayout;
		pInfo.renderPass = pipelineInfo->renderPass;
		pInfo.stageCount = 2;
		pInfo.pVertexInputState = vertexInputInfo;
		pInfo.pInputAssemblyState = &iaInfo;
		pInfo.pRasterizationState = &rInfo;
		pInfo.pColorBlendState = &cbInfo;
		pInfo.pMultisampleState = &msInfo;
		pInfo.pViewportState = &vInfo;
		pInfo.pDepthStencilState = &dsInfo;
		pInfo.pStages = sInfo;
		pInfo.pDynamicState = &dInfo;

		VkPipeline pipeline;
		error = vkCreateGraphicsPipelines(logicalDevice, pipelineInfo->pipelineCache, 1, &pInfo, nullptr, &pipeline);
		Assert(error, "could not create pipeline");
		return pipeline;
	}

	VkDescriptorPool PrepareDescriptorPool(VkDevice logicalDevice)
	{
		VkDescriptorPoolSize typeCount[1];
		typeCount[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		typeCount[0].descriptorCount = 1;

		VkDescriptorPoolCreateInfo dpInfo = {};
		dpInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		dpInfo.poolSizeCount = 1;
		dpInfo.pPoolSizes = typeCount;
		dpInfo.maxSets = 1;
		VkDescriptorPool descriptorPool;
		VkResult error = vkCreateDescriptorPool(logicalDevice, &dpInfo, nullptr, &descriptorPool);
		Assert(error, "could not create descriptor pool");
		return descriptorPool;

	}

	VkDescriptorSet NewDescriptorSet(VkDevice logicalDevice, const PipelineInfo* pipelineInfo, VkDescriptorBufferInfo uniformBufferInfo)
	{

		VkDescriptorSetAllocateInfo dsInfo = {};
		dsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		dsInfo.descriptorPool = pipelineInfo->descriptorPool;
		dsInfo.descriptorSetCount = 1;
		dsInfo.pSetLayouts = &pipelineInfo->descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkResult error = vkAllocateDescriptorSets(logicalDevice, &dsInfo, &descriptorSet);
		Assert(error, "could not allocate descriptor set");

		//binding 0: uniform buffer
		VkWriteDescriptorSet wdSet = {};
		wdSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wdSet.dstSet = descriptorSet;
		wdSet.descriptorCount = 1;
		wdSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		wdSet.pBufferInfo = &uniformBufferInfo;
		wdSet.dstBinding = 0;
		vkUpdateDescriptorSets(logicalDevice, 1, &wdSet, 0, nullptr);
		return descriptorSet;

	}

	void BuildCmdBuffers(const DeviceInfo* deviceInfo, const PipelineInfo* pipelineInfo, const SwapchainInfo* swapchainInfo, const VertexBuffer* vertexBuffer, uint32_t clientWidth, uint32_t clientHeight)
	{
		VkCommandBufferBeginInfo cbInfo = {};
		cbInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		VkClearValue clearVals[2] = {};
		clearVals[0].color = { {0.2f, 0.2f, 0.2f, 1.0f} };
		clearVals[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo rbInfo = {};
		rbInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rbInfo.renderPass = pipelineInfo->renderPass;
		rbInfo.renderArea.offset = { 0, 0 };
		rbInfo.renderArea.extent = { clientWidth, clientHeight };
		rbInfo.clearValueCount = 2;
		rbInfo.pClearValues = clearVals;

		VkResult error;
		for (uint32_t i = 0; i < deviceInfo->drawCmdBuffers.size(); i++)
		{
			VkFramebuffer currFrame = deviceInfo->frameBuffers[i];
			VkCommandBuffer currCmd = deviceInfo->drawCmdBuffers[i];

			rbInfo.framebuffer = currFrame;

			error = vkBeginCommandBuffer(currCmd, &cbInfo);
			Assert(error, "could not begin command buffer");

			vkCmdBeginRenderPass(currCmd, &rbInfo, VK_SUBPASS_CONTENTS_INLINE);

			//update dynamic states
			VkViewport vp = {};
			vp.height = (float)clientHeight;
			vp.width = (float)clientWidth;
			vp.minDepth = 0.0f;
			vp.maxDepth = 1.0f;
			vkCmdSetViewport(currCmd, 0, 1, &vp);

			VkRect2D scissor = {};
			scissor.extent = { clientWidth, clientHeight };
			scissor.offset = { 0, 0 };
			vkCmdSetScissor(currCmd, 0, 1, &scissor);

			//bind descriptor sets describing shader bind points
			vkCmdBindDescriptorSets(currCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineInfo->pipelineLayout, 0, 1, &pipelineInfo->descriptorSet, 0, nullptr);

			//bind pipeline, including shaders
			vkCmdBindPipeline(currCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineInfo->pipeline);

			//bind triangle vertices
			VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(currCmd, VERTEX_BUFFER_BIND_ID, 1, &vertexBuffer->vBuffer, offsets);

			//bind triangle indicies
			vkCmdBindIndexBuffer(currCmd, vertexBuffer->iBuffer, 0, VK_INDEX_TYPE_UINT32);

			//draw indexed triangle
			vkCmdDrawIndexed(currCmd, vertexBuffer->iCount, 1, 0, 0, 1);

			vkCmdEndRenderPass(currCmd);

			//add a present memory barrier
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
			barrier.image = swapchainInfo->images[i];

			vkCmdPipelineBarrier(currCmd,
				VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1,
				&barrier);
			error = vkEndCommandBuffer(currCmd);
			Assert(error, "could not end draw command buffers");
		}

	}

	void Init(MainMemory* m)
	{

		m->consoleHandle = GetConsoleWindow();
		//ShowWindow(m->consoleHandle, SW_HIDE);
		m->camera = NewCamera();
		m->timerInfo = NewTimerInfo();

		m->windowInfo = NewWindowInfo(EXE_NAME, &m->input, 1200, 800);
		m->input = NewInputInfo(&m->windowInfo);
		//ShowWindow(m->windowHandle, SW_HIDE);


		NewInstanceInfo(m->windowInfo.appName, &m->instanceInfo);
		NewPhysDeviceInfo(m->instanceInfo.vkInstance, &m->physDeviceInfo);

		NewSurfaceInfo(&m->windowInfo, &m->instanceInfo, &m->physDeviceInfo, &m->surfaceInfo);
		NewDeviceInfo(&m->windowInfo, &m->physDeviceInfo, &m->surfaceInfo, &m->deviceInfo);
		NewSwapchainInfo(&m->windowInfo, &m->physDeviceInfo, &m->surfaceInfo, &m->deviceInfo, &m->swapchainInfo);
		SetupCommandBuffers(&m->deviceInfo, m->swapchainInfo.imageCount);

		setupDepthStencil(&m->deviceInfo,
			&m->physDeviceInfo,
			m->windowInfo.clientWidth,
			m->windowInfo.clientHeight);

		m->pipelineInfo.renderPass = NewRenderPass(m->deviceInfo.device,
			m->surfaceInfo.colorFormat,
			m->physDeviceInfo.supportedDepthFormat);

		m->pipelineInfo.pipelineCache = NewPipelineCache(m->deviceInfo.device);
		m->deviceInfo.frameBuffers = NewFrameBuffer(m->deviceInfo.device,
			&m->swapchainInfo.views,
			m->pipelineInfo.renderPass,
			m->deviceInfo.depthStencil.view,
			m->swapchainInfo.imageCount,
			m->windowInfo.clientWidth,
			m->windowInfo.clientHeight);
		//TODO why does the setup cmd buffer need to be flushed and recreated?
		FlushSetupCommandBuffer(&m->deviceInfo);
		m->deviceInfo.setupCmdBuffer = NewSetupCommandBuffer(m->deviceInfo.device, m->deviceInfo.cmdPool);
		//create cmd buffer for image barriers and converting tilings
		//TODO what are tilings?
		//m->textureCmdBuffer = NewCommandBuffer(m->deviceInfo.device, m->deviceInfo.cmdPool);

		m->vertexBuffer.vPos =
		{
			{ { 1.0f, 1.0f, 0.0f  },{ 1.0f, 0.0f, 0.0f } },
			{ { -1.0f, 1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
			{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
		};
		m->vertexBuffer.iPos = { 0, 1, 2 };
		m->vertexBuffer.iCount = 3;
		PrepareVertexData(&m->deviceInfo, m->physDeviceInfo.memoryProperties, &m->vertexBuffer);
		PrepareCameraBuffers(m->deviceInfo.device,
			m->physDeviceInfo.memoryProperties,
			&m->camera,
			m->windowInfo.clientWidth,
			m->windowInfo.clientHeight,
			-2.0f,
			v3(40.0f, 0.0f, 0.0f));

		m->pipelineInfo.descriptorSetLayout = NewDescriptorSetLayout(m->deviceInfo.device, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		m->pipelineInfo.pipelineLayout = NewPipelineLayout(m->deviceInfo.device, m->pipelineInfo.descriptorSetLayout);
		m->pipelineInfo.pipeline = NewPipeline(m->deviceInfo.device, &m->pipelineInfo, &m->vertexBuffer.viInfo);
		m->pipelineInfo.descriptorPool = PrepareDescriptorPool(m->deviceInfo.device);
		m->pipelineInfo.descriptorSet = NewDescriptorSet(m->deviceInfo.device, &m->pipelineInfo, m->camera.desc);
		BuildCmdBuffers(&m->deviceInfo, &m->pipelineInfo, &m->swapchainInfo, &m->vertexBuffer, m->windowInfo.clientWidth, m->windowInfo.clientHeight);

	}


	void Render(const DeviceInfo* deviceInfo, SwapchainInfo* swapchainInfo)
	{
		vkDeviceWaitIdle(deviceInfo->device);
		VkResult error;
		error = AcquireNextImage(deviceInfo, swapchainInfo);
		Assert(error, "could not acquire next image in update and render");

		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		barrier.image = swapchainInfo->images[swapchainInfo->currentBuffer];

		VkCommandBufferBeginInfo bInfo = {};
		bInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		error = vkBeginCommandBuffer(deviceInfo->postPresentCmdBuffer, &bInfo);
		Assert(error, "could not being post present buffer in update and render");

		vkCmdPipelineBarrier(deviceInfo->postPresentCmdBuffer,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkSubmitInfo sInfo = {};
		sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sInfo.commandBufferCount = 1;
		sInfo.pCommandBuffers = &deviceInfo->postPresentCmdBuffer;

		error = vkEndCommandBuffer(deviceInfo->postPresentCmdBuffer);
		Assert(error, "could not end post present cmd buffer in update and render");

		error = vkQueueSubmit(deviceInfo->queue, 1, &sInfo, nullptr);
		Assert(error, "could not submit queue in update and render");

		error = vkQueueWaitIdle(deviceInfo->queue);
		Assert(error, "could not queue wait idle in update and render");

		VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		sInfo = {};
		sInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		sInfo.pWaitDstStageMask = &stageFlags;
		sInfo.waitSemaphoreCount = 1;
		sInfo.pWaitSemaphores = &deviceInfo->presentComplete;
		sInfo.commandBufferCount = 1;
		sInfo.pCommandBuffers = &deviceInfo->drawCmdBuffers[swapchainInfo->currentBuffer];
		sInfo.signalSemaphoreCount = 1;
		sInfo.pSignalSemaphores = &deviceInfo->renderComplete;

		error = vkQueueSubmit(deviceInfo->queue, 1, &sInfo, nullptr);
		Assert(error, "could not submit cmd buffer in update and render");

		error = QueuePresent(deviceInfo, swapchainInfo);
		Assert(error, "could not present queue in update and render");

		vkDeviceWaitIdle(deviceInfo->device);

	}

	void Update(MainMemory* m)
	{
		InputInfo input = m->input;
		float speed = CAMERA_SPEED * m->timerInfo.frameTimeMilliSec;
		if (input.keys[keyW])
		{
			m->camera.cameraPos.position += m->camera.cameraPos.front * speed * 10.0f;
		}
		if (input.keys[keyS])
		{
			m->camera.cameraPos.position -= m->camera.cameraPos.front * speed* 10.0f;
		}
		if (input.keys[keyA])
		{
			m->camera.cameraPos.position -= m->camera.cameraPos.right * speed* 10.0f;
		}
		if (input.keys[keyD])
		{
			m->camera.cameraPos.position += m->camera.cameraPos.right * speed* 10.0f;
		}
		if (input.mouseInWindow)
		{
			if (input.keys[lMouse])
			{
				//mouse is inside clientrect and player is holding down left mouse button
				float xOffset = input.lastMousePos.x - input.mousePos.x;
				float yOffset = input.mousePos.y - input.lastMousePos.y;
				m->camera.cameraPos.yaw += xOffset * speed / 10;
				m->camera.cameraPos.pitch -= yOffset * speed / 10;
			}

		}

		UpdateCamera(m->deviceInfo.device, m->camera, m->windowInfo.clientWidth, m->windowInfo.clientHeight);

	}

	void PollEvents(const WindowInfo* windowInfo)
	{
		MSG msg;
#if 1
		while (PeekMessage(&msg, windowInfo->windowHandle, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#else
		for (int i = 0; i < 20; i++)
		{
			if (!PeekMessage(&msg, windInfo->windowHandle, 0, 0, PM_REMOVE))
			{
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
#endif
	}

	void Quit(MainMemory* m)
	{
		//destroy app specific buffers etc
		vkDestroyBuffer(m->deviceInfo.device, m->vertexBuffer.vBuffer, nullptr);
		vkFreeMemory(m->deviceInfo.device, m->vertexBuffer.vMemory, nullptr);

		vkDestroyBuffer(m->deviceInfo.device, m->vertexBuffer.iBuffer, nullptr);
		vkFreeMemory(m->deviceInfo.device, m->vertexBuffer.iMemory, nullptr);

		vkDestroyBuffer(m->deviceInfo.device, m->camera.buffer, nullptr);
		vkFreeMemory(m->deviceInfo.device, m->camera.memory, nullptr);


		//destroy "framework" objects
		DestroyTimerInfo(&m->timerInfo);
		DestroyWindowInfo(&m->windowInfo);
		DestroyPipelineInfo(m->deviceInfo.device, &m->pipelineInfo);
		DestroySwapchainInfo(&m->deviceInfo, &m->swapchainInfo);
		DestroySurfaceInfo(&m->instanceInfo, &m->surfaceInfo);
		DestroyDeviceInfo(&m->deviceInfo);
		DestroyInstanceInfo(&m->instanceInfo);
	}


	int main(int argv, char** argc)
	{

		MainMemory* m = new MainMemory();
		Init(m);
		while (m->input.running)
		{
			Tick(&m->timerInfo);
			PollEvents(&m->windowInfo);
			Update(m);
			Render(&m->deviceInfo, &m->swapchainInfo);
			Tock(&m->timerInfo);
			Sleep(&m->timerInfo, 30);
		}
		Quit(m);
		delete m;
		return 0;
	}


