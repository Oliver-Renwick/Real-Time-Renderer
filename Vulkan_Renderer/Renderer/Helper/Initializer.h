#pragma once

#include<vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <vector>
#include <fstream>
#include <assert.h>
#include <iostream>
#include <array>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

#define VK_CHECK_RESULT(f)																				\
{																										\
	VkResult res = (f);																					\
	if (res != VK_SUCCESS)																				\
	{																									\
		std::cout << "Fatal : VkResult is \"" << " Failed " << "\" in " << __FILE__ << " at line " << __LINE__ << "\n"; \
		assert(res == VK_SUCCESS);																		\
	}																									\
}



static VkShaderModule createShaderModule(const std::vector<char>& code, VkDevice device)
{
	VkShaderModule module;

	VkShaderModuleCreateInfo shaderInfo{};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = code.size();
	shaderInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VK_CHECK_RESULT(vkCreateShaderModule(device, &shaderInfo, nullptr, &module));
	return module;
}




static std::vector<char> readfile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (file.is_open())
	{
		size_t filesize = (size_t)file.tellg();
		std::vector<char> buffer(filesize);

		file.seekg(0);
		file.read(buffer.data(), filesize);

		file.close();

		return buffer;
	}

	std::vector<char> temp;
	return temp;
}

inline VkPipelineShaderStageCreateInfo createshaderStage(const std::string& filename, VkShaderStageFlagBits shaderstage, VkDevice device)
{
	if (filename.empty()) { throw std::runtime_error("The Given file is empty or corrupt"); }

	auto code = readfile(filename);

	VkShaderModule _module = createShaderModule(code, device);

	VkPipelineShaderStageCreateInfo vertexshaderInfo{};
	vertexshaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexshaderInfo.pName = "main";
	vertexshaderInfo.module = _module;
	vertexshaderInfo.stage = shaderstage;

	return vertexshaderInfo;

}

inline std::string png_to_ktx(std::string filename)
{
	size_t pos = filename.find_last_of(".");
	std::string res = filename.substr(0, pos);
	res = "ktx_files/" + res + ".ktx";

	return res;
}

namespace initializer
{

	inline std::string getProjectPath()
	{
		std::string s = PROJECT_PATH;
		return s;
	}

	inline VkSpecializationInfo specializationInfo(const std::vector<VkSpecializationMapEntry>& mapEntries, size_t dataSize, const void* data)
	{
		VkSpecializationInfo specializationInfo{};
		specializationInfo.mapEntryCount = static_cast<uint32_t>(mapEntries.size());
		specializationInfo.pMapEntries = mapEntries.data();
		specializationInfo.dataSize = dataSize;
		specializationInfo.pData = data;
		return specializationInfo;
	}


	inline bool fileExist(const std::string& filename)
	{
		std::ifstream f(filename.c_str());
		return !f.fail();
	}

	inline VkSemaphoreCreateInfo semaphoreInfo()
	{
		VkSemaphoreCreateInfo semaphoreCI{};
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		return semaphoreCI;
	}

	inline VkRect2D rect2D(uint32_t width, uint32_t height, uint32_t OffsetX, uint32_t OffsetY)
	{
		VkRect2D rectInfo{};
		rectInfo.extent.height = height;
		rectInfo.extent.width = width;
		rectInfo.offset.x = OffsetX;
		rectInfo.offset.y = OffsetY;

		return rectInfo;
	}
	inline VkViewport viewPort(float width, float height, float minDepth, float maxDepth)
	{
		VkViewport viewPortInfo{};
		viewPortInfo.width = width;
		viewPortInfo.height = height;
		viewPortInfo.minDepth = minDepth;
		viewPortInfo.maxDepth = maxDepth;

		return viewPortInfo;
	}

	inline VkRenderPassBeginInfo renderPassBeginInfo()
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

		return renderPassInfo;
	}

	inline VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo()
	{
		VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		return vertexInputStateCI;
	}

	inline VkVertexInputAttributeDescription vertexInputAttributeDescription(
		uint32_t binding,
		uint32_t location,
		VkFormat format,
		uint32_t offset
	)
	{
		VkVertexInputAttributeDescription vertexinputAttributeInfo{};
		vertexinputAttributeInfo.binding = binding;
		vertexinputAttributeInfo.format = format;
		vertexinputAttributeInfo.location = location;
		vertexinputAttributeInfo.offset = offset;

		return vertexinputAttributeInfo;
	}

	inline VkVertexInputBindingDescription vertexInputBinding(
		uint32_t binding,
		uint32_t stride,
		VkVertexInputRate inputRate
	)
	{
		VkVertexInputBindingDescription vertexInputBinding;
		vertexInputBinding.stride = stride;
		vertexInputBinding.binding = binding;
		vertexInputBinding.inputRate = inputRate;

		return vertexInputBinding;
	}


	inline VkGraphicsPipelineCreateInfo CreateGraphicsPipelineInfo()
	{
		VkGraphicsPipelineCreateInfo graphicspipelineCI{};
		graphicspipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

		graphicspipelineCI.basePipelineIndex = -1;
		graphicspipelineCI.basePipelineHandle = VK_NULL_HANDLE;

		return graphicspipelineCI;
	}

	inline VkGraphicsPipelineCreateInfo CreateGraphicsPipelineInfo(
		VkPipelineLayout pipelineLayout,
		VkRenderPass renderPass,
		VkPipelineCreateFlags flags = 0
	)
	{
		VkGraphicsPipelineCreateInfo graphicspipelineCI{};
		graphicspipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicspipelineCI.layout = pipelineLayout;
		graphicspipelineCI.flags = flags;
		graphicspipelineCI.renderPass = renderPass;

		graphicspipelineCI.basePipelineIndex = -1;
		graphicspipelineCI.basePipelineHandle = VK_NULL_HANDLE;

		return graphicspipelineCI;
	}

	inline VkPipelineTessellationStateCreateInfo tessilationInfo(
		uint32_t pathControlPoint
	)
	{
		VkPipelineTessellationStateCreateInfo tessilationstateCI{};
		tessilationstateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
		tessilationstateCI.patchControlPoints = pathControlPoint;
		return tessilationstateCI;
	}

	inline VkPipelineDynamicStateCreateInfo dynamicStateInfo(
		const VkDynamicState* dynamicState,
		uint32_t dynamicStateCount,
		VkPipelineDynamicStateCreateFlags flags = 0
	)
	{
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.flags = flags;
		dynamicStateCI.dynamicStateCount = dynamicStateCount;
		dynamicStateCI.pDynamicStates = dynamicState;
		return dynamicStateCI;
	}

	inline VkPipelineDynamicStateCreateInfo dynamicStateInfo(
		const std::vector<VkDynamicState>& dynamicState,
		VkPipelineDynamicStateCreateFlags flags = 0
	)
	{
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.flags = flags;
		dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicState.size());
		dynamicStateCI.pDynamicStates = dynamicState.data();
		return dynamicStateCI;
	}

	inline VkPipelineMultisampleStateCreateInfo MultiSampleStateInfo(
		VkSampleCountFlagBits rasterizationSamples,
		VkPipelineMultisampleStateCreateFlags flags = 0
	)
	{
		VkPipelineMultisampleStateCreateInfo multiSampleCI{};
		multiSampleCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multiSampleCI.rasterizationSamples = rasterizationSamples;
		multiSampleCI.flags = flags;

		return multiSampleCI;
	}

	inline VkPipelineViewportStateCreateInfo viewportStateInfo(
		uint32_t viewportCount,
		uint32_t scissorCount,
		VkViewport* _viewport,
		VkRect2D* _scissors,
		VkPipelineViewportStateCreateFlags flag = 0
	)
	{
		VkPipelineViewportStateCreateInfo viewPortStateCI{};
		viewPortStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewPortStateCI.scissorCount = scissorCount;
		viewPortStateCI.viewportCount = viewportCount;
		viewPortStateCI.pScissors = _scissors;
		viewPortStateCI.pViewports = _viewport;
		viewPortStateCI.flags = flag;
		return viewPortStateCI;
	}

	inline VkPipelineDepthStencilStateCreateInfo depthStencilInfo(
		VkBool32 depthTestEnable,
		VkBool32 depthWriteEnable,
		VkCompareOp depthCompare
	)
	{
		VkPipelineDepthStencilStateCreateInfo depthStencilCI{};
		depthStencilCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilCI.depthTestEnable = depthTestEnable;
		depthStencilCI.depthWriteEnable = depthWriteEnable;
		depthStencilCI.depthCompareOp = depthCompare;
		depthStencilCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		return depthStencilCI;
	}

	inline VkPipelineColorBlendStateCreateInfo colorBlendInfo(
		uint32_t attachmentCount,
		VkPipelineColorBlendAttachmentState* blendAttachment
	)
	{
		VkPipelineColorBlendStateCreateInfo colorBlendCI{};
		colorBlendCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendCI.attachmentCount = attachmentCount;
		colorBlendCI.pAttachments = blendAttachment;
		colorBlendCI.logicOpEnable = VK_FALSE;

		return colorBlendCI;
	}

	inline VkPipelineColorBlendAttachmentState colorBlendAttachment(
		VkColorComponentFlags colorWriteMask,
		VkBool32 blendEnable
	)
	{
		VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo{};
		colorBlendAttachmentInfo.colorWriteMask = colorWriteMask;
		colorBlendAttachmentInfo.blendEnable = blendEnable;

		return colorBlendAttachmentInfo;
	}

	inline VkPipelineRasterizationStateCreateInfo razterizationState(
		VkPolygonMode polygonMode,
		VkCullModeFlags cullMode,
		VkFrontFace frontFace,
		VkPipelineRasterizationStateCreateFlags flag = 0
	)
	{
		VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
		rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationInfo.cullMode = cullMode;
		rasterizationInfo.flags = flag;
		rasterizationInfo.frontFace = frontFace;
		rasterizationInfo.polygonMode = polygonMode;
		rasterizationInfo.depthClampEnable = VK_FALSE;
		rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
		rasterizationInfo.lineWidth = 1.0f;

		return rasterizationInfo;
	}

	inline VkPipelineInputAssemblyStateCreateInfo inputAssemblySate(
		VkPrimitiveTopology primitiveTopoloy,
		VkBool32 primitiverestartEnable,
		VkPipelineInputAssemblyStateCreateFlags flag
	)
	{
		VkPipelineInputAssemblyStateCreateInfo stateInfo{};
		stateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		stateInfo.topology = primitiveTopoloy;
		stateInfo.primitiveRestartEnable = primitiverestartEnable;
		stateInfo.flags = flag;

		return stateInfo;
	}


	inline VkPushConstantRange pushConstantRange(
		VkShaderStageFlags shaderflag,
		uint32_t size,
		uint32_t offset
	)
	{
		VkPushConstantRange pushConstantCI{};
		pushConstantCI.stageFlags = shaderflag;
		pushConstantCI.size = size;
		pushConstantCI.offset = offset;

		return pushConstantCI;
	}

	inline VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo(
		VkDescriptorSetLayout* setLayout,
		uint32_t layoutCount
	)
	{
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.pSetLayouts = setLayout;
		pipelineLayoutCI.setLayoutCount = layoutCount;
		return pipelineLayoutCI;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet descriptorSet,
		VkDescriptorType type,
		uint32_t binding,
		VkDescriptorBufferInfo* bufferInfo,
		uint32_t descriptorCount = 1
	)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.pNext = nullptr;
		writeDescriptorSet.descriptorType = type;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = descriptorCount;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.pBufferInfo = bufferInfo;
		return writeDescriptorSet;

	}

	inline VkSpecializationMapEntry specializationMapEntry(uint32_t constantID, uint32_t offset, size_t size)
	{
		VkSpecializationMapEntry specializationMapEntry{};
		specializationMapEntry.constantID = constantID;
		specializationMapEntry.offset = offset;
		specializationMapEntry.size = size;
		return specializationMapEntry;
	}

	inline VkWriteDescriptorSet writeDescriptorSet(
		VkDescriptorSet descriptorSet,
		VkDescriptorType type,
		uint32_t binding,
		VkDescriptorImageInfo* imageInfo,
		uint32_t descriptorCount = 1
	)
	{
		VkWriteDescriptorSet writeDescriptorSet{};
		writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescriptorSet.descriptorType = type;
		writeDescriptorSet.dstSet = descriptorSet;
		writeDescriptorSet.descriptorCount = descriptorCount;
		writeDescriptorSet.dstBinding = binding;
		writeDescriptorSet.pImageInfo = imageInfo;
		return writeDescriptorSet;

	}


	inline VkDescriptorSetAllocateInfo descriptorSetAllocInfo(
		VkDescriptorSetLayout* setLayout,
		VkDescriptorPool Pool,
		uint32_t setCount
	)
	{
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pSetLayouts = setLayout;
		allocInfo.descriptorPool = Pool;
		allocInfo.descriptorSetCount = setCount;

		return allocInfo;
	}

	inline VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* layoutbinding, uint32_t binding_count)
	{
		VkDescriptorSetLayoutCreateInfo descriptorLayoutCI{};
		descriptorLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutCI.bindingCount = binding_count;
		descriptorLayoutCI.pBindings = layoutbinding;
		return descriptorLayoutCI;
	}

	inline VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(
		VkDescriptorType descriptorType,
		VkShaderStageFlags shaderFlag,
		uint32_t binding,
		uint32_t descriptorCount = 1
	)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.stageFlags = shaderFlag;
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = descriptorCount;
		return layoutBinding;

	}

	inline VkDescriptorPoolCreateInfo descriptorPoolCreateInfo(const std::vector<VkDescriptorPoolSize>& poolsize, uint32_t max_set)
	{
		VkDescriptorPoolCreateInfo poolCI{};
		poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCI.poolSizeCount = static_cast<uint32_t>(poolsize.size());
		poolCI.pPoolSizes = poolsize.data();
		poolCI.maxSets = max_set;

		return poolCI;
	}

	inline VkDescriptorPoolSize descriptorPoolSize(VkDescriptorType descriptorType, uint32_t count)
	{
		VkDescriptorPoolSize descriptorPoolsizeInfo{};
		descriptorPoolsizeInfo.descriptorCount = count;
		descriptorPoolsizeInfo.type = descriptorType;

		return descriptorPoolsizeInfo;
	}
	inline VkFenceCreateInfo fenceCreateInfo(VkFenceCreateFlags flag = 0)
	{
		VkFenceCreateInfo fenceCI{};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.flags = flag;
		return fenceCI;
	}
	inline VkSubmitInfo submitInfo()
	{
		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		return info;
	}

	inline VkCommandBufferBeginInfo CmdBufferBeginInfo()
	{
		VkCommandBufferBeginInfo cmdBufferInfo{};
		cmdBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		return cmdBufferInfo;
	}

	inline VkCommandBufferAllocateInfo CmdBufferAllocateInfo(VkCommandPool pool, VkCommandBufferLevel level, uint32_t bufferCount)
	{
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandBufferCount = bufferCount;
		bufferInfo.commandPool = pool;
		bufferInfo.level = level;

		return bufferInfo;
	}

	inline VkBufferCreateInfo bufferCreateInfo()
	{
		VkBufferCreateInfo bufCreateInfo{};
		bufCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		return bufCreateInfo;
	}
	inline void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask)
	{
		// Create an image barrier object
		VkImageMemoryBarrier imageMemoryBarrier{};
		imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.oldLayout = oldImageLayout;
		imageMemoryBarrier.newLayout = newImageLayout;
		imageMemoryBarrier.image = image;
		imageMemoryBarrier.subresourceRange = subresourceRange;

		// Source layouts (old)
		// Source access mask controls actions that have to be finished on the old layout
		// before it will be transitioned to the new layout
		switch (oldImageLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Image layout is undefined (or does not matter)
			// Only valid as initial layout
			// No flags required, listed only for completeness
			imageMemoryBarrier.srcAccessMask = 0;
			break;

		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Image is preinitialized
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image is a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image is a depth/stencil attachment
			// Make sure any writes to the depth/stencil buffer have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image is a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image is a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image is read by a shader
			// Make sure any shader reads from the image have been finished
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		// Target layouts (new)
		// Destination access mask controls the dependency for the new image layout
		switch (newImageLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Image will be used as a transfer destination
			// Make sure any writes to the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Image will be used as a transfer source
			// Make sure any reads from the image have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// Image will be used as a color attachment
			// Make sure any writes to the color buffer have been finished
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// Image layout will be used as a depth/stencil attachment
			// Make sure any writes to depth/stencil buffer have been finished
			imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Image will be read in a shader (sampler, input attachment)
			// Make sure any writes to the image have been finished
			if (imageMemoryBarrier.srcAccessMask == 0)
			{
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;
		default:
			// Other source layouts aren't handled (yet)
			break;
		}

		vkCmdPipelineBarrier(
			cmdbuffer,
			srcStageMask,
			dstStageMask,
			0,
			0, nullptr,
			0, nullptr,
			1, &imageMemoryBarrier);
	}

	inline void setImageLayout(
		VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageAspectFlags aspectMask,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask)
	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;
		setImageLayout(cmdbuffer, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);
	}

	inline VkBufferCreateInfo bufferCreateInfo(VkBufferUsageFlags usage, VkDeviceSize size)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		return bufferInfo;
	}


	inline VkMemoryAllocateInfo MemoryAllocateInfo()
	{
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		return allocInfo;
	}

	inline VkMappedMemoryRange MappedMemoryRanges()
	{
		VkMappedMemoryRange mappedRange{};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		return mappedRange;
	}
}