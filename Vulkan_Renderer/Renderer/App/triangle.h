#pragma once
#include <Renderer/Vulkan_Renderer.h>
#include "Renderer/Helper/Vulkan_Buffer.h"
#include "Renderer/Helper/Initializer.h"
#include "Renderer/Helper/Model_loader.h"
#include "Renderer/Helper/Texture.h"
#include "Renderer/Skybox.h"
#include "Renderer/UI/UI.h"

#include "Renderer/Helper/Camera.h"


namespace tde
{

	class triangle
	{
	public:
		triangle();
		~triangle();


		void Connect(Renderer* renderer);
		void CreateDepthBufferResource();
		void CreateColorBufferResource();
		void CreateGraphicsPipelineLayout();
		void CreateGraphicsPipeline();
		void CreateRenderPass();
		void CreateFrameBuffer();
		void AllocateCommandBuffer();
		void CreatePrimitiveObject();
		void CreateDescriptorSetLayout();
		void CreateUniformBuffer();
		void UpdateUniformBuffer(uint32_t currentImage);
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void CreateUIObject();
		void ReCreateSwapchain();

		void Load_Asset();

		void RecordCommandBuffer(VkCommandBuffer cmdBuffer, uint32_t imageIndex);
		void draw();

		void prepare();
		void Destroy();

	private:
		Renderer* m_renderer;
		VkRenderPass m_renderpass = VK_NULL_HANDLE;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		std::vector<VkFramebuffer> frameBuffers;
		std::vector<VkCommandBuffer> m_commandBuffer;
		Skybox cubemap;
		
		//Synchronization Objects
		std::vector<VkSemaphore> imageAvailableSemaphore;
		std::vector<VkSemaphore> renderFinishedSemaphore;
		std::vector<VkFence> inFlightFence;

		//Pipelines
		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkPipeline m_wireframePipeline = VK_NULL_HANDLE;
		bool m_wireframe = false;

		//Descriptor Objects
		struct
		{
			VkDescriptorSetLayout matrices = VK_NULL_HANDLE;
			VkDescriptorSetLayout textures = VK_NULL_HANDLE;

		}m_descriptorSetLayout;

		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

		const int MAX_FRAMES_IN_FLIGHT = 2;
		uint32_t currentFrame = 0;

		//Uniform Buffer
		VkBuffer m_uniformBuffer;
		VkDeviceMemory m_uniformBufferMemory;
		void* uniformBufferMapped;

		//Depth attachments
		VkImage depthImage;
		VkDeviceMemory depthImageMemory;
		VkImageView depthImageView;

		//Color attachments
		VkImage colorImage;
		VkDeviceMemory colorImageMemory;
		VkImageView colorImageView;

		//Camera
		Camera camera;

		//Formats
		VkFormat depthFormat;
		VkFormat colorFormat;

		//UI
		UI Gui_Overlay;

		//Models
	private:
		Vulkan_gltfModel sponza_model;
		std::vector<Vulkan_gltfModel::Vertex> VertexBuffer;
		std::vector<uint32_t> IndexBuffer;;
		

		struct UniformBufferObject {
			glm::mat4 view;
			glm::mat4 proj;
			glm::vec4 lightPos = glm::vec4(0.0f, 2.5f, 0.0f, 1.0f);
			glm::vec4 viewpos;
		};

		
	};

}