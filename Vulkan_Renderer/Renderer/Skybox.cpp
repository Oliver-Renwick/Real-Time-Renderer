#include "Skybox.h"

namespace tde
{
	Skybox::Skybox()
	{
	}

	Skybox::~Skybox()
	{
		this->Destroy();
	}

	void Skybox::Destroy()
	{
		vkDestroyPipeline(m_renderer->getDevice(), skyboxPipeline, nullptr);
		vkFreeMemory(m_renderer->getDevice(), imageMemory, nullptr);
		vkDestroyImageView(m_renderer->getDevice(), m_imageView, nullptr);
		vkDestroyImage(m_renderer->getDevice(), m_image, nullptr);
		vkDestroySampler(m_renderer->getDevice(), m_sampler, nullptr);
	}

	void Skybox::loadCubemap(std::string filename, VkFormat format, const Renderer* renderer)
	{
		ktxResult result;
		ktxTexture* _ktxTexture;
		
		if (!initializer::fileExist(filename))
		{
			throw std::runtime_error("Unable to Open The Cubemap");
		}

		result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &_ktxTexture);

		assert(result == VK_SUCCESS);

		this->m_renderer = renderer;
		this->m_height = _ktxTexture->baseHeight;
		this->m_width = _ktxTexture->baseWidth;
		this->m_miplevel = _ktxTexture->numLevels;

		ktx_uint8_t* textureData = ktxTexture_GetData(_ktxTexture);
		ktx_size_t textureSize = ktxTexture_GetDataSize(_ktxTexture);

		VkMemoryRequirements memReqs{};
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;

		//Staging Buffer 
		VkBuffer StagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo BufferInfo{};
		BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferInfo.size = textureSize;

		BufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		
		VK_CHECK_RESULT(vkCreateBuffer(m_renderer->getDevice(), &BufferInfo, nullptr, &StagingBuffer));

		vkGetBufferMemoryRequirements(m_renderer->getDevice(), StagingBuffer, &memReqs);

		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = m_renderer->findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		
		VK_CHECK_RESULT(vkAllocateMemory(m_renderer->getDevice(), &allocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_renderer->getDevice(), StagingBuffer, stagingMemory, 0));

		uint8_t* data;
		vkMapMemory(m_renderer->getDevice(), stagingMemory, 0, memReqs.size, 0, (void**)&data);
		memcpy(data, textureData, textureSize);
		vkUnmapMemory(m_renderer->getDevice(), stagingMemory);

		//Image Create Info
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.format = format;
		imageInfo.mipLevels = m_miplevel;
		imageInfo.extent.width = m_width;
		imageInfo.extent.height = m_height;
		imageInfo.extent.depth = 1;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		
		//count as many faces of Cubemap
		imageInfo.arrayLayers = 6;

		//This flag is required for Cubemap Images
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		VK_CHECK_RESULT(vkCreateImage(m_renderer->getDevice(), &imageInfo, nullptr, &m_image));

		vkGetImageMemoryRequirements(m_renderer->getDevice(), m_image, &memReqs);

		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = m_renderer->findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		
		VK_CHECK_RESULT(vkAllocateMemory(m_renderer->getDevice(), &allocInfo, nullptr, &imageMemory));
		VK_CHECK_RESULT(vkBindImageMemory(m_renderer->getDevice(), m_image, imageMemory, 0));

		VkCommandBuffer copycmd = m_renderer->beginSingleCommand();

		//SetupBuffer Copy Region 
		std::vector<VkBufferImageCopy> bufferCopyRegions;

		for (uint32_t face = 0; face < 6; face++)
		{
			for (uint32_t level = 0; level < m_miplevel; level++)
			{
				ktx_size_t offset;
				KTX_error_code ret = ktxTexture_GetImageOffset(_ktxTexture, level, 0, face, &offset);
				assert(ret == KTX_SUCCESS);

				VkBufferImageCopy bufferCopy{};
				bufferCopy.imageSubresource.baseArrayLayer = face;
				bufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				bufferCopy.imageSubresource.mipLevel = level;
				bufferCopy.imageSubresource.layerCount = 1;
				bufferCopy.imageExtent.width = _ktxTexture->baseWidth >> level;
				bufferCopy.imageExtent.height = _ktxTexture->baseHeight >> level;
				bufferCopy.imageExtent.depth = 1;
				bufferCopy.bufferOffset = offset;

				bufferCopyRegions.push_back(bufferCopy);
			}
		}

		// Image barrier for optimal image (target)
		// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
		VkImageSubresourceRange subResourceRange{};
		subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResourceRange.baseMipLevel = 0;
		subResourceRange.levelCount = m_miplevel;
		subResourceRange.layerCount = 6;

		initializer::setImageLayout(
			copycmd,
			m_image,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subResourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);

		vkCmdCopyBufferToImage(
			copycmd,
			StagingBuffer,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			static_cast<uint32_t>(bufferCopyRegions.size()),
			bufferCopyRegions.data()
		);

		m_imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		initializer::setImageLayout(
			copycmd,
			m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			m_imageLayout,
			subResourceRange,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
		);


		m_renderer->endSingleCommand(copycmd);  

		//Vulkan Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = static_cast<float>(m_miplevel);
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.maxAnisotropy = 1.0f;

		if (m_renderer->device_features.samplerAnisotropy)
		{
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = m_renderer->device_properties.limits.maxSamplerAnisotropy;
		}

		VK_CHECK_RESULT(vkCreateSampler(m_renderer->getDevice(), &samplerInfo, nullptr, &m_sampler));

		//Image View
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.format = format;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;

		imageViewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

		//For all Six Faces
		imageViewInfo.subresourceRange.layerCount = 6;
		//For mip levels
		imageViewInfo.subresourceRange.levelCount = m_miplevel;
		imageViewInfo.image = m_image;

		VK_CHECK_RESULT(vkCreateImageView(m_renderer->getDevice(), &imageViewInfo, nullptr, &m_imageView));

		vkDestroyBuffer(m_renderer->getDevice(), StagingBuffer, nullptr);
		vkFreeMemory(m_renderer->getDevice(), stagingMemory, nullptr);
		ktxTexture_Destroy(_ktxTexture);

		updateDescriptor();
	}
	

	void Skybox::preparePipelines(VkPipelineLayout pipelineLayout, VkRenderPass renderPass)
	{
		//Create Pipeline for Skybox
		auto vertexShader = readfile(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/skybox_vert.spv");
		auto fragmentShader = readfile(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/skybox_frag.spv");

		VkShaderModule vertexModule = createShaderModule(vertexShader, m_renderer->getDevice());
		VkShaderModule fragmentModule = createShaderModule(fragmentShader, m_renderer->getDevice());

		VkPipelineShaderStageCreateInfo vertexPipelineStage{};
		vertexPipelineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexPipelineStage.module = vertexModule;
		vertexPipelineStage.pName = "main";
		vertexPipelineStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
		
		VkPipelineShaderStageCreateInfo fragmentPipelineStage{};
		fragmentPipelineStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentPipelineStage.module = fragmentModule;
		fragmentPipelineStage.pName = "main";
		fragmentPipelineStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

		VkPipelineShaderStageCreateInfo shaderstage[] = { vertexPipelineStage, fragmentPipelineStage };

		const std::vector<VkVertexInputBindingDescription> vertexBindingDescription{
			initializer::vertexInputBinding(0, sizeof(Vulkan_gltfModel::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};

		const std::vector<VkVertexInputAttributeDescription> vertexAttributeDescription{
			initializer::vertexInputAttributeDescription(0, 0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, pos)),
			initializer::vertexInputAttributeDescription(0, 1,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, normal)),
			initializer::vertexInputAttributeDescription(0, 2,VK_FORMAT_R32G32_SFLOAT,    offsetof(Vulkan_gltfModel::Vertex, uv)),
			initializer::vertexInputAttributeDescription(0, 3,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, color)),
			initializer::vertexInputAttributeDescription(0, 4,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, tangent))
		};

		VkPipelineVertexInputStateCreateInfo vertexInputCI = initializer::vertexInputStateCreateInfo();
		vertexInputCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBindingDescription.size());
		vertexInputCI.pVertexBindingDescriptions = vertexBindingDescription.data();
		vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescription.size());
		vertexInputCI.pVertexAttributeDescriptions = vertexAttributeDescription.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblystateCI = initializer::inputAssemblySate(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0);
		VkPipelineRasterizationStateCreateInfo rasterizationstateCI = initializer::razterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, 0);
		VkPipelineColorBlendAttachmentState colorBlendAttacments = initializer::colorBlendAttachment(0xf, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorBlendCI = initializer::colorBlendInfo(1, &colorBlendAttacments);
		VkPipelineDepthStencilStateCreateInfo depthstencilCI = initializer::depthStencilInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);
		VkPipelineViewportStateCreateInfo viewPortInfo = initializer::viewportStateInfo(1, 1, nullptr, nullptr);
		VkPipelineMultisampleStateCreateInfo multiSampleCI = initializer::MultiSampleStateInfo(m_renderer->maxSampleCount());
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicStateCI = initializer::dynamicStateInfo(dynamicStateEnables);
		
		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.renderPass = renderPass;
		pipelineCI.layout = pipelineLayout;
		pipelineCI.pInputAssemblyState = &inputAssemblystateCI;
		pipelineCI.pRasterizationState = &rasterizationstateCI;
		pipelineCI.pColorBlendState = &colorBlendCI;
		pipelineCI.pDepthStencilState = &depthstencilCI;
		pipelineCI.pViewportState = &viewPortInfo;
		pipelineCI.pMultisampleState = &multiSampleCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderstage;
		pipelineCI.pVertexInputState = &vertexInputCI;

		rasterizationstateCI.cullMode = VK_CULL_MODE_FRONT_BIT;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_renderer->getDevice(), nullptr, 1, &pipelineCI, nullptr, &skyboxPipeline));


	}

	void Skybox::updateDescriptor()
	{
		descriptorInfo.imageInfo.imageLayout = m_imageLayout;
		descriptorInfo.imageInfo.imageView = m_imageView;
		descriptorInfo.imageInfo.sampler = m_sampler;
	}

	void Skybox::Draw_Skybox(VkCommandBuffer cmdBuffer, VkPipelineLayout layout)
	{
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, skyboxPipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 1, 1, &descriptorInfo.descriptorSet, 0, nullptr);
		skybox_model.draw(cmdBuffer, layout);
	}
}