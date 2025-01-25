#pragma once

#include "Renderer/Vulkan_Renderer.h"
#include "Initializer.h"

namespace tde
{
	static void createImage(uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits msaaSample, VkImageTiling tiling, VkImageUsageFlags usage, 
		VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory, const Renderer* renderer)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.usage = usage;
		imageInfo.extent.height = height;
		imageInfo.extent.width = width;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = format;
		imageInfo.tiling = tiling;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = msaaSample;

		VK_CHECK_RESULT(vkCreateImage(renderer->getDevice(), &imageInfo, nullptr, &image));

		//Memory Requirement
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(renderer->getDevice(), image, &memReqs);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = renderer->findMemoryIndex(memReqs.memoryTypeBits, properties);

		VK_CHECK_RESULT(vkAllocateMemory(renderer->getDevice(), &allocInfo, nullptr, &memory));

		vkBindImageMemory(renderer->getDevice(), image, memory, 0);
	}

	static void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory, const Renderer* renderer)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;

		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = usage;

		VK_CHECK_RESULT(vkCreateBuffer(renderer->getDevice(), &bufferInfo, nullptr, &buffer));

		//Memory Requirements
		VkMemoryRequirements memreqs;
		vkGetBufferMemoryRequirements(renderer->getDevice(), buffer, &memreqs);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.memoryTypeIndex = renderer->findMemoryIndex(memreqs.memoryTypeBits, properties);
		allocInfo.allocationSize = memreqs.size;

		VK_CHECK_RESULT(vkAllocateMemory(renderer->getDevice(), &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(renderer->getDevice(), buffer, bufferMemory, 0);
	}
}
