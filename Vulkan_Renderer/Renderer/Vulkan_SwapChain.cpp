#include "Vulkan_SwapChain.h"

#include <cstdint>
#include <limits>
#include <algorithm>

namespace tde
{
	Swapchain::Swapchain()
	{

	}

	Swapchain::~Swapchain()
	{

	}

	void Swapchain::connect(const VkInstance instance, const VkPhysicalDevice physicalDevice, const VkDevice device,const VkSurfaceKHR surface, const int& width, const int& height)
	{
		m_instance = instance;
		m_physicalDevice = physicalDevice;
		m_device = device;
		m_surface = surface;

		this->m_height = height;
		this->m_width = width;

	}

	Swapchain::SwapChainSupportDetails Swapchain::queryswapchain(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails swapchain_details;

		//capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapchain_details.capabilities);

		//Format
		uint32_t format_count = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, nullptr);
		if (format_count != 0)
		{
			swapchain_details.formats.resize(format_count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &format_count, swapchain_details.formats.data());
		}

		//Present Mode
		uint32_t present_count = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_count, nullptr);

		if (present_count != 0)
		{
			swapchain_details.presentModes.resize(present_count);
			vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &present_count, swapchain_details.presentModes.data());
		}

		return swapchain_details;
	}

	VkSurfaceFormatKHR Swapchain::chooseswapsurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormat)
	{
		for (const auto& format : availableFormat)
		{
			if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return format;
			}
		}

		return availableFormat[0];
	}

	VkPresentModeKHR Swapchain::choosepresentMode(const std::vector<VkPresentModeKHR>& availablePresent)
	{
		for (const auto& present : availablePresent)
		{
			if (present == VK_PRESENT_MODE_MAILBOX_KHR)
				return present;
		}

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Swapchain::chooseswapextent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		VkExtent2D extent = {};

		if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)())
		{
			return capabilities.currentExtent;
		}
		else
		{
			extent.width = static_cast<uint32_t>(m_width);
			extent.height = static_cast<uint32_t>(m_height);

			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
		}

		return extent;
	}

	void Swapchain::CreateSwapchain()
	{
		SwapChainSupportDetails swapchainsupport = queryswapchain(m_physicalDevice, m_surface);

		VkSurfaceFormatKHR surface_format = chooseswapsurfaceFormat(swapchainsupport.formats);
		VkPresentModeKHR present_mode = choosepresentMode(swapchainsupport.presentModes);
		VkExtent2D extent = chooseswapextent(swapchainsupport.capabilities);


		uint32_t image_count = swapchainsupport.capabilities.minImageCount + 1;

		if (swapchainsupport.capabilities.maxImageCount > 0 && image_count > swapchainsupport.capabilities.maxImageCount)
		{
			image_count = swapchainsupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR swapchainCI{};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.surface = m_surface;

		swapchainCI.minImageCount = image_count;
		swapchainCI.imageFormat = surface_format.format;
		swapchainCI.imageColorSpace = surface_format.colorSpace;
		swapchainCI.imageExtent = extent;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		//Swapchain Transformation
		swapchainCI.preTransform = swapchainsupport.capabilities.currentTransform;

		//Blending of other window systems
		swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		swapchainCI.presentMode = present_mode;
		swapchainCI.clipped = VK_TRUE;

		//To-Do : implementaion in swapchain recreation
		swapchainCI.oldSwapchain = VK_NULL_HANDLE;

		VkResult res = vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapchain);

		if (res != VK_SUCCESS)
			throw std::runtime_error("Cannot able to create swapchain");

		swapchainImageFormat = surface_format.format;
		swapchainextent = extent;

		vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, nullptr);
		swapchain_Images.resize(image_count);
		vkGetSwapchainImagesKHR(m_device, m_swapchain, &image_count, swapchain_Images.data());

		std::cout << "Image Count : " << image_count << std::endl;
	}

	void Swapchain::CreateImageView()
	{
		swapchain_imageView.resize(swapchain_Images.size());

		for (size_t i = 0; i < swapchain_Images.size(); i++)
		{
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = swapchain_Images[i];

			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = swapchainImageFormat;

			viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;

			VK_CHECK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &swapchain_imageView[i]));
		}
	}

	void Swapchain::cleanup()
	{
		for (auto& imageview : swapchain_imageView)
		{
			vkDestroyImageView(m_device, imageview, nullptr);
		}

		vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	}
}