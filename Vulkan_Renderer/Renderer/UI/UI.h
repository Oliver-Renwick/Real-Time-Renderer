#pragma once

#include "imgui.h"
#include <vulkan/vulkan.h>
#include <iostream>

#include "Renderer/Helper/Initializer.h"

namespace tde
{
	class UI
	{
	public:

		UI() = default;
		~UI() = default;

		void Init();
		void prepareResources();
		void preparePipeline(const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits sampleCount);
		void Draw(VkCommandBuffer commandBuffer);
		bool Update();
		void Destroy();

	public:
		VkDevice m_device = VK_NULL_HANDLE;
		VkQueue m_queue = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkCommandPool m_cmdPool = VK_NULL_HANDLE;
		std::vector<VkPipelineShaderStageCreateInfo> shaders;

	private:
		uint32_t findMemoryIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties);
		VkCommandBuffer beginSingleCommand();
		void endSingleCommand(VkCommandBuffer cmdBuffer);
		void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
			VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);


	private:
		VkImage fontImage;
		VkDeviceMemory fontMemory;
		VkImageView fontView;
		VkSampler fontSampler;

		struct
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			uint32_t count = 0;
			void* mapped = nullptr;
		}vertexBuffer;

		struct {
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			uint32_t count = 0;
			void* mapped = nullptr;
		}indexBuffer;

		struct
		{
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE ;
			VkDescriptorSetLayout setLayout =  VK_NULL_HANDLE ;
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
			VkPipeline pipeline = VK_NULL_HANDLE;
		}ui;

	private:
		float scale{ 1.0f };
		struct pushConstantBlock
		{
			glm::vec2 scale;
			glm::vec2 translate;
		}pushBlock;
	};
}