#include "UI.h"

namespace tde
{

	void UI::Init()
	{
		//ImGui Init
		ImGui::CreateContext();

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);;
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.7f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
		style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8);


		//Dimensions
		ImGuiIO& io = ImGui::GetIO();
		io.FontGlobalScale = scale;
	}

	void UI::prepareResources()
	{
		ImGuiIO& io = ImGui::GetIO();

		//Create Font Texture
		unsigned char* fontData;
		int texWidth, texHeight;

		const std::string fileName = "C:/Users/ASUS/Desktop/Vulkan_Renderer/Vulkan_Renderer/Asset/Roboto-Medium.ttf";
		io.Fonts->AddFontFromFileTTF(fileName.c_str(), 16.0f * scale);

		io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

		VkDeviceSize uploadSize = texHeight * texWidth * 4  * sizeof(char);

		ImGuiStyle& style = ImGui::GetStyle();
		style.ScaleAllSizes(scale);

		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.extent.width = texWidth;
		imageInfo.extent.height = texHeight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		
		VK_CHECK_RESULT(vkCreateImage(m_device, &imageInfo, nullptr, &fontImage));
		VkMemoryRequirements memReqs{};
		VkMemoryAllocateInfo allocInfo{};
		vkGetImageMemoryRequirements(m_device, fontImage, &memReqs);

		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.memoryTypeIndex = findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		allocInfo.allocationSize = memReqs.size;
		
		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &fontMemory));
		VK_CHECK_RESULT(vkBindImageMemory(m_device, fontImage, fontMemory, 0));

		//Image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = fontImage;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.layerCount = 1;
		VK_CHECK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &fontView));  

		//Staging Buffer for font Data Upload ;- First finish buffer requirment and build buffer class by yourself
		VkBuffer stagingBuffer{};
		VkDeviceMemory stagingMemory{};

		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferInfo.size = uploadSize;
		
		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &stagingBuffer));

		
		vkGetBufferMemoryRequirements(m_device, stagingBuffer, &memReqs);
		allocInfo.pNext = nullptr;
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = findMemoryIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(m_device, stagingBuffer, stagingMemory, 0));

		void* data;
		vkMapMemory(m_device, stagingMemory, 0, memReqs.size, 0, &data);
		memcpy(data, fontData, uploadSize);
		vkUnmapMemory(m_device, stagingMemory);

		//Copy Buffer data to font Image

		VkCommandBuffer copyCmd = beginSingleCommand();

		initializer::setImageLayout(copyCmd, fontImage, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

		VkBufferImageCopy bufferCopy{};
		bufferCopy.imageSubresource.layerCount = 1;
		bufferCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopy.imageExtent.width = texWidth;
		bufferCopy.imageExtent.height = texHeight;
		bufferCopy.imageExtent.depth = 1;

		vkCmdCopyBufferToImage(copyCmd, stagingBuffer, fontImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopy);
		
		initializer::setImageLayout(copyCmd, fontImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

		endSingleCommand(copyCmd);

		vkDestroyBuffer(m_device, stagingBuffer, nullptr);
		vkFreeMemory(m_device, stagingMemory, nullptr);

		//Font Texture Sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		VK_CHECK_RESULT(vkCreateSampler(m_device, &samplerInfo, nullptr, &fontSampler));

		//Descriptor Pool
		std::vector<VkDescriptorPoolSize> poolSize{
			initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		poolInfo.pPoolSizes = poolSize.data();
		poolInfo.maxSets = 2;
		VK_CHECK_RESULT(vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &ui.descriptorPool));

		//Descriptor Set Layout
		std::vector<VkDescriptorSetLayoutBinding> layoutbinding = {
			initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0)
		};
		VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
		setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setLayoutInfo.bindingCount = static_cast<uint32_t>(layoutbinding.size());
		setLayoutInfo.pBindings = layoutbinding.data();
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_device, &setLayoutInfo, nullptr, &ui.setLayout));

		//DescriptorSets
		VkDescriptorSetAllocateInfo setallocInfo{};
		setallocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		setallocInfo.descriptorSetCount = 1;
		setallocInfo.descriptorPool = ui.descriptorPool;
		setallocInfo.pSetLayouts = &ui.setLayout;
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device, &setallocInfo, &ui.descriptorSet));

		VkDescriptorImageInfo descriptorInfo{ fontSampler, fontView , VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			initializer::writeDescriptorSet(ui.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &descriptorInfo)
		};
		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

	}

	void UI::preparePipeline(const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat, VkSampleCountFlagBits sampleCount)
	{
		assert(renderPass);

		//Pipeline Layout
		VkPushConstantRange pushRange = initializer::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(pushConstantBlock), 0);
		VkPipelineLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutInfo.setLayoutCount = 1;
		layoutInfo.pSetLayouts = &ui.setLayout;
		layoutInfo.pushConstantRangeCount = 1;
		layoutInfo.pPushConstantRanges = &pushRange;
		VK_CHECK_RESULT(vkCreatePipelineLayout(m_device, &layoutInfo, nullptr, &ui.pipelineLayout));

		//Graphics Pipeline
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI =
			initializer::inputAssemblySate(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0);
		
		VkPipelineRasterizationStateCreateInfo rasterizationCI =
			initializer::razterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);

		//Enable Blending
		VkPipelineColorBlendAttachmentState colorBlendattachment{};
		colorBlendattachment.blendEnable = VK_TRUE;
		colorBlendattachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendattachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendattachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendattachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendattachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendattachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendattachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendCI = initializer::colorBlendInfo(1, &colorBlendattachment);

		VkPipelineDepthStencilStateCreateInfo depthStencilCI = initializer::depthStencilInfo(VK_FALSE, VK_FALSE, VK_COMPARE_OP_ALWAYS);

		VkPipelineViewportStateCreateInfo viewPortCI = initializer::viewportStateInfo(1, 1, nullptr, nullptr);

		VkPipelineMultisampleStateCreateInfo multisampleCI = initializer::MultiSampleStateInfo(sampleCount);

		std::vector<VkDynamicState> dynamicStateEnables =
		{
			VK_DYNAMIC_STATE_SCISSOR,
			VK_DYNAMIC_STATE_VIEWPORT
		};

		VkPipelineDynamicStateCreateInfo dynamicStageCI =
			initializer::dynamicStateInfo(dynamicStateEnables);

		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.layout = ui.pipelineLayout;
		pipelineCI.renderPass = renderPass;
		pipelineCI.subpass = 0;

		pipelineCI.pInputAssemblyState = &inputAssemblyCI;
		pipelineCI.pRasterizationState = &rasterizationCI;
		pipelineCI.pColorBlendState = &colorBlendCI;
		pipelineCI.pDepthStencilState = &depthStencilCI;
		pipelineCI.pViewportState = &viewPortCI;
		pipelineCI.pMultisampleState = &multisampleCI;
		pipelineCI.pDynamicState = &dynamicStageCI;
		pipelineCI.stageCount = static_cast<uint32_t>(shaders.size());
		pipelineCI.pStages = shaders.data();

		// Vertex bindings an attributes based on ImGui vertex definition
		std::vector<VkVertexInputBindingDescription> vertexInputBindings = {
			initializer::vertexInputBinding(0, sizeof(ImDrawVert), VK_VERTEX_INPUT_RATE_VERTEX)
		};

		std::vector<VkVertexInputAttributeDescription> vertexAttributes = {
			initializer::vertexInputAttributeDescription(0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)),
			initializer::vertexInputAttributeDescription(0, 1, VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)),
			initializer::vertexInputAttributeDescription(0, 2, VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)),
		};

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributes.size());
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindings.size());
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();

		pipelineCI.pVertexInputState = &vertexInputInfo;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipelineCI, nullptr, &ui.pipeline));
	}

	bool UI::Update()
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		bool updateCmdBuffer = false;

		if (!imDrawData) { return false; }

		VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
		VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);


		if (indexBufferSize == 0 || vertexBufferSize == 0)
		{
			return false;
		}

		if (vertexBuffer.buffer == VK_NULL_HANDLE || vertexBuffer.count != imDrawData->TotalVtxCount)
		{
			// Un Map if its mapped
			if (vertexBuffer.mapped)
			{
				vkUnmapMemory(m_device, vertexBuffer.memory);
				vertexBuffer.mapped = nullptr;
			}

			//Destroy information on creation of new resources
			if (vertexBuffer.buffer)
			{
				vkDestroyBuffer(m_device, vertexBuffer.buffer, nullptr);
			}
			if (vertexBuffer.memory)
			{
				vkFreeMemory(m_device, vertexBuffer.memory, nullptr);
			}

			createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBuffer.buffer,
				vertexBuffer.memory);
			vertexBuffer.count = imDrawData->TotalVtxCount;

			if (vertexBuffer.mapped)
			{
				vkUnmapMemory(m_device, vertexBuffer.memory);
				vertexBuffer.mapped = nullptr;
			}

			vkMapMemory(m_device, vertexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &vertexBuffer.mapped);
			updateCmdBuffer = true;

		}


		if (indexBuffer.buffer == VK_NULL_HANDLE || indexBuffer.count < imDrawData->TotalIdxCount)
		{
			// Un Map if its mapped
			if (indexBuffer.mapped)
			{
				vkUnmapMemory(m_device, indexBuffer.memory);
				indexBuffer.mapped = nullptr;
			}

			//Destroy information on creation of new resources
			if (indexBuffer.buffer)
			{
				vkDestroyBuffer(m_device, indexBuffer.buffer, nullptr);
			}
			if (indexBuffer.memory)
			{
				vkFreeMemory(m_device, indexBuffer.memory, nullptr);
			}

			createBuffer(vertexBufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBuffer.buffer,
				indexBuffer.memory);
			indexBuffer.count = imDrawData->TotalIdxCount;

			vkMapMemory(m_device, indexBuffer.memory, 0, VK_WHOLE_SIZE, 0, &indexBuffer.mapped);
			updateCmdBuffer = true;

		}

		//Upload Data
		ImDrawVert* vtxDst = (ImDrawVert*)vertexBuffer.mapped;
		ImDrawIdx* IdxDst = (ImDrawIdx*)indexBuffer.mapped;

		for (int i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
			memcpy(IdxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
			vtxDst += cmd_list->VtxBuffer.Size;
			IdxDst += cmd_list->IdxBuffer.Size;
		}

		//!!!!Learn what Mapped Memory Range is!!!!!!
		VkMappedMemoryRange mappedRange = {};
		mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		mappedRange.memory = vertexBuffer.memory;
		mappedRange.offset = 0;
		mappedRange.size = VK_WHOLE_SIZE;
		vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);

		mappedRange.memory = indexBuffer.memory;
		vkFlushMappedMemoryRanges(m_device, 1, &mappedRange);

		return updateCmdBuffer;
	}

	void UI::Draw(VkCommandBuffer commandBuffer)
	{
		ImDrawData* imDrawData = ImGui::GetDrawData();
		int vertexOffset = 0;
		int indexOffset = 0;

		if ((!imDrawData) || (imDrawData->CmdListsCount == 0))
		{
			return;
		}

		ImGuiIO& io = ImGui::GetIO();

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui.pipeline);
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ui.pipelineLayout, 0, 1, &ui.descriptorSet, 0, nullptr);

		pushBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		pushBlock.translate = glm::vec2(-1.0f);
		vkCmdPushConstants(commandBuffer, ui.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pushConstantBlock), &pushBlock);

		VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT16);

		for (int32_t i = 0; i < imDrawData->CmdListsCount; i++)
		{
			const ImDrawList* cmd_list = imDrawData->CmdLists[i];
			for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++)
			{
				const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
				VkRect2D scissorRect;
				scissorRect.offset.x = std::max((int32_t)(pcmd->ClipRect.x), 0);
				scissorRect.offset.y = std::max((int32_t)(pcmd->ClipRect.y), 0);
				scissorRect.extent.width = (uint32_t)(pcmd->ClipRect.z - pcmd->ClipRect.x);
				scissorRect.extent.height = (uint32_t)(pcmd->ClipRect.w - pcmd->ClipRect.y);
				vkCmdSetScissor(commandBuffer, 0, 1, &scissorRect);
				vkCmdDrawIndexed(commandBuffer, pcmd->ElemCount, 1, indexOffset, vertexOffset, 0);
				indexOffset += pcmd->ElemCount;
			}
			vertexOffset += cmd_list->VtxBuffer.Size;
		}
	}

	void UI::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
		VkBufferCreateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;

		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = usage;

		VK_CHECK_RESULT(vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer));

		//Memory Requirements
		VkMemoryRequirements memreqs;
		vkGetBufferMemoryRequirements(m_device, buffer, &memreqs);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.memoryTypeIndex = findMemoryIndex(memreqs.memoryTypeBits, properties);
		allocInfo.allocationSize = memreqs.size;

		VK_CHECK_RESULT(vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
	}

	VkCommandBuffer UI::beginSingleCommand()
	{
		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = m_cmdPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &cmdBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		return cmdBuffer;
	}

	void UI::endSingleCommand(VkCommandBuffer cmdBuffer)
	{
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(m_queue);

		vkFreeCommandBuffers(m_device, m_cmdPool, 1, &cmdBuffer);
	}

	uint32_t UI::findMemoryIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties)
	{
		VkPhysicalDeviceMemoryProperties memProperties{};
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("Cannot able to fin Memory Index");
	}

	void UI::Destroy()
	{
		
		vkDestroyBuffer(m_device, vertexBuffer.buffer, nullptr);
		vkFreeMemory(m_device, vertexBuffer.memory, nullptr);
		vkDestroyBuffer(m_device, indexBuffer.buffer, nullptr);
		vkFreeMemory(m_device, indexBuffer.memory, nullptr);

		vkDestroySampler(m_device, fontSampler, nullptr);
		vkDestroyImageView(m_device, fontView, nullptr);
		vkDestroyImage(m_device, fontImage, nullptr);

		vkDestroyPipeline(m_device, ui.pipeline, nullptr);
		vkFreeDescriptorSets(m_device, ui.descriptorPool, 1, &ui.descriptorSet);
		vkDestroyDescriptorSetLayout(m_device, ui.setLayout, nullptr);
		vkDestroyDescriptorPool(m_device, ui.descriptorPool, nullptr);

		vkDestroyPipelineLayout(m_device, ui.pipelineLayout, nullptr);


		if (ImGui::GetCurrentContext())
		{
			ImGui::DestroyContext();
		}

		//To Do Destroy Resources
	}
}