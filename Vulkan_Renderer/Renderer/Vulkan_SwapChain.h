#pragma once
#include "vulkan/vulkan.h"
#include <windows.h>
#include "Helper/Initializer.h"
#include <assert.h>
#include <iostream>
#include <vector>


namespace tde
{
	class Swapchain
	{
	public:
		Swapchain();
		~Swapchain();

		void connect(const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device, const VkSurfaceKHR surface, const int& width, const int& height);
		void CreateSwapchain();
		void CreateImageView();
		void cleanup();

		const VkFormat getSwapchainImageFormat() const { return swapchainImageFormat; }
		const VkExtent2D getSwapchainExtent() const { return swapchainextent; }
		const VkSwapchainKHR getSwapChain() const { return m_swapchain; }

		std::vector<VkImageView> swapchain_imageView;

	private:
		VkExtent2D chooseswapextent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkPresentModeKHR choosepresentMode(const std::vector<VkPresentModeKHR>& availablePresent);
		VkSurfaceFormatKHR chooseswapsurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormat);

	public:
		VkInstance m_instance = VK_NULL_HANDLE;
		VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
		VkDevice m_device = VK_NULL_HANDLE;
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;

		int m_width = 0;
		int m_height = 0;

		std::vector<VkImage> swapchain_Images;
		VkFormat swapchainImageFormat;
		VkExtent2D swapchainextent;

	public:
		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};

		SwapChainSupportDetails queryswapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
	};
}