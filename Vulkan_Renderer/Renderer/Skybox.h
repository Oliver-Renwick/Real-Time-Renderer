#pragma once
#include "Renderer/Vulkan_Renderer.h"
#include "Renderer/Helper/Model_loader.h"
#include "Renderer/Helper/Initializer.h"

namespace tde
{
	class Skybox
	{
	public:
		Skybox();
		~Skybox();

		void loadCubemap(std::string filename, VkFormat format, const Renderer* renderer);
		void preparePipelines(VkPipelineLayout pipelineLayout, VkRenderPass renderPass);
		void Draw_Skybox(VkCommandBuffer cmdBuffer, VkPipelineLayout layout);
		void updateDescriptor();

	public:
		Vulkan_gltfModel skybox_model;
		

	private:
		void Destroy();
		 
		VkPipeline skyboxPipeline = VK_NULL_HANDLE;
		const Renderer* m_renderer;

	private:
		Texture cubeMap;

		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_miplevel = 0;

		VkImage m_image;
		VkImageView m_imageView;
		VkSampler m_sampler;

		VkDeviceMemory imageMemory;

		VkImageLayout m_imageLayout;

	private:
		struct skyBox_Vertex
		{
			glm::vec3 pos;
		};

	public:

		struct
		{
			VkDescriptorImageInfo imageInfo;
			VkDescriptorSet descriptorSet;

		}descriptorInfo;

	};
}