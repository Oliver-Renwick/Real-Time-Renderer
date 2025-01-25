#pragma once

#include "Renderer/Helper/Initializer.h"
#include "Renderer/Vulkan_Renderer.h"




namespace tde
{
	class Texture
	{
	public:
		const Renderer*       m_renderer;
		VkImage               m_image;
		VkImageView           m_imageView;
		VkSampler             m_sampler;
		VkImageLayout         m_imageLayout;
		VkDeviceMemory        m_deviceMemory;
		uint32_t              m_height, m_width;
		uint32_t              m_miplevels;
		uint32_t              m_layerCount;
		VkDescriptorImageInfo m_descriptorimageInfo;

		void updateDescriptor();
		void Destroy();

		ktxResult loadKtxFile(std::string filename, ktxTexture** target);
	};

	class Texture2D : public Texture
	{
	public:
		void loadFromFile(
			std::string        filename,
			VkFormat           format,
			const Renderer*    renderer,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			bool               forceLiner = false);


		void FromBuffer(
			void* buffer,
			VkDeviceSize       bufferSize,
			VkFormat           format,
			uint32_t           texWidth,
			uint32_t           texHeight,
			const Renderer*    renderer,
			VkFilter           filter = VK_FILTER_LINEAR,
			VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
			VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	};
}