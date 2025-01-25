#pragma once

#include <vector>
#include <string>
#include <optional>
#include <set>

#include "Windows/_window.h"
#include <Windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include "Vulkan_SwapChain.h"
#include "Helper/Initializer.h"
#include "Helper/Debugger.h"


namespace tde
{
	class Renderer
	{
	public:
		Renderer(Win* v_win);
		~Renderer();
		
		void Init();

		const void ReCreateSwapchain();

	public:

		const VkDevice getDevice() const { return m_Device; }
		const Swapchain getSwapchain() const { return m_swapchain; }
		const VkPhysicalDevice getPhysicalDevice() const { return m_PhysicalDevice; }
		const VkQueue getGraphicsQueue() const { return graphics_queue; }
		const VkQueue getPresentQueue() const { return present_queue; }
		const Win* getWindow() const { return m_win; }
		const VkCommandPool getCommandPool() const { return m_commandPool; }
		const VkSampleCountFlagBits maxSampleCount() const { return m_msaaSamples; }
		const VkPipelineCache getPipelineCache() const { return m_pipelineCache; }


	public:
		VkFormat findDepthFormat() const {
			return findSupportedFormt({ 
				VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT },
				VK_IMAGE_TILING_OPTIMAL,
				VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT); }
		bool hasStencilComponent(VkFormat format)const { return format == VK_FORMAT_D32_SFLOAT_S8_UINT || 
			format == VK_FORMAT_D24_UNORM_S8_UINT; }

		VkSampleCountFlagBits getMaxUsableSampleCount();

	private:

		void Vulkan_Init();
		void Vulkan_Instance();
		void Vulkan_PhysicalDevice();
		void Vulkan_Device();
		void Window_Surface();
		void CreateCommandPool();
		void CreatePipelineCache();

	private:
		void prepare();
		void Init_Swapchain();
		void Destroy();

	public:
		uint32_t findMemoryIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const ;
		VkCommandBuffer beginSingleCommand() const ;
		void endSingleCommand(VkCommandBuffer cmdBuffer) const ;
		void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const ;
		void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldlayout, VkImageLayout newLayout) const;
		void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)const;
		VkImageView CreateimageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlag) const;
		VkFormat findSupportedFormt(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
	private:
		bool IsDeviceSuitable(const VkPhysicalDevice physicalDevice);
		void set_upConsole();
		bool checkDeviceExtensionSupport(const VkPhysicalDevice device);
		bool checkValidationLayerSupport();


	private:
		Win* m_win = nullptr;
		VkInstance m_Instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_PhysicalDevice = VK_NULL_HANDLE;
		VkDevice m_Device = VK_NULL_HANDLE;
		VkQueue graphics_queue;
		VkQueue present_queue;
		VkSurfaceKHR surface;
		Swapchain m_swapchain;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkPipelineCache m_pipelineCache = VK_NULL_HANDLE;

	private:
		std::vector<std::string> supportedInstanceExtension;
	
	public:
		VkPhysicalDeviceProperties device_properties;
		VkPhysicalDeviceFeatures device_features;
		bool bindlessSupport = false;

	public:
		struct QueueFamiles
		{
			std::optional<uint32_t> graphics_queue;
			std::optional<uint32_t> present_queue;

			bool isComplete()
			{
				return graphics_queue.has_value() && present_queue.has_value();
			}
		};
		QueueFamiles findQueueFamily (const VkPhysicalDevice physicalDevice) const ;


	private:
		const std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
		const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};


	public:
		RECT rect;
		int m_width = 0;
		int m_height = 0;
		VkSampleCountFlagBits m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
		
	};
}