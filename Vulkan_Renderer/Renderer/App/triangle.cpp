#include "triangle.h"

#include "stb_image.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

bool check = true;


namespace tde
{
	triangle::triangle()
	{
	}

	triangle::~triangle()
	{
		this->Destroy();
	}

	void triangle::Connect(Renderer* renderer)
	{
		this->m_renderer = renderer;

		int c_width = m_renderer->m_width;
		int c_height = m_renderer->m_height;
		camera.type = Camera::CameraType::Keyboard;

		camera.SetPosition(glm::vec3(-4.2f, -1.26f, 0.0f));
		camera.SetRotation(glm::vec3(0.0f, -90.0f, 0.0f));
		camera.setPrespective(glm::radians(45.0f), float(c_width) / float(c_height), 0.1f, 256.0f);
		camera.UpdateViewMatrix();
	}

	void triangle::prepare()
	{
		Load_Asset();
		CreateDepthBufferResource();
		CreateColorBufferResource();
		CreateRenderPass();
		CreateFrameBuffer();
		CreateDescriptorSetLayout();
		CreateGraphicsPipelineLayout();
		CreateGraphicsPipeline();
		CreateUIObject();
		CreateUniformBuffer();
		CreateDescriptorPool();
		CreateDescriptorSets();
		AllocateCommandBuffer();
		CreatePrimitiveObject();
	}

	void triangle::CreateUIObject()
	{
		//Create Ui Object  
		Gui_Overlay.Init();
		Gui_Overlay.m_cmdPool = m_renderer->getCommandPool();
		Gui_Overlay.m_device = m_renderer->getDevice();
		Gui_Overlay.m_physicalDevice = m_renderer->getPhysicalDevice();
		Gui_Overlay.m_queue = m_renderer->getGraphicsQueue();

		Gui_Overlay.shaders = {
			createshaderStage(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/UI_vert.spv", VK_SHADER_STAGE_VERTEX_BIT, m_renderer->getDevice()),
			createshaderStage(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/UI_frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, m_renderer->getDevice())
		};

		Gui_Overlay.prepareResources();
		Gui_Overlay.preparePipeline(m_renderpass, colorFormat, depthFormat, m_renderer->maxSampleCount());


	}


	void triangle::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding setLayoutBinding{};
		setLayoutBinding.binding = 0;
		setLayoutBinding.descriptorCount = 1;
		setLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setLayoutBinding.pImmutableSamplers = nullptr;
		setLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		//matrices
		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { setLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();

		if (vkCreateDescriptorSetLayout(m_renderer->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout.matrices) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}


		std::vector<VkDescriptorSetLayoutBinding> textureLayoutBinding =
		{
			//color
			initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0),
			initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1),
			initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2)

		};

		layoutInfo.bindingCount = static_cast<uint32_t>(textureLayoutBinding.size());
		layoutInfo.pBindings = textureLayoutBinding.data();
		  
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_renderer->getDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout.textures));
	}

	void triangle::Load_Asset()
	{
		cubemap.skybox_model.LoadFromFile( initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Models/cube.gltf", m_renderer);
		sponza_model.LoadFromFile(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Models/sponza/sponza.gltf", m_renderer);
		cubemap.loadCubemap(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Textures/cubemap_vulkan.ktx", VK_FORMAT_R8G8B8A8_SRGB, m_renderer);
	}

	void triangle::CreateUniformBuffer()
	{
		VkDeviceSize buffersize = sizeof(UniformBufferObject);

	
		createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_uniformBuffer, m_uniformBufferMemory, m_renderer);

		vkMapMemory(m_renderer->getDevice(), m_uniformBufferMemory, 0, buffersize, 0, &uniformBufferMapped);
	
	}

	void triangle::CreateRenderPass()
	{

		//Color attachments
		VkAttachmentDescription colorAttachments{};
		colorAttachments.samples = m_renderer->maxSampleCount();
		colorAttachments.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachments.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachments.format = m_renderer->getSwapchain().getSwapchainImageFormat();

		colorAttachments.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachments.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

		colorAttachments.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachments.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//Depth Attachments
		VkAttachmentDescription depthAttachment{};
		depthAttachment.format = m_renderer->findDepthFormat();
		depthAttachment.samples = m_renderer->maxSampleCount();
		depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//Color Attachment Resolve
		VkAttachmentDescription colorAttachmentResolve{};
		colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachmentResolve.format = m_renderer->getSwapchain().getSwapchainImageFormat();
		colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		//Color Attachment Reference
		VkAttachmentReference colorattachmentRef{};
		colorattachmentRef.attachment = 0;
		colorattachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		//Depth Attachment Reference
		VkAttachmentReference depthAttachmentRef{};
		depthAttachmentRef.attachment = 1;
		depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		//Color Attachment resolve Reference
		VkAttachmentReference colorAttachmentResolveRef{};
		colorAttachmentResolveRef.attachment = 2;
		colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorattachmentRef;
		subpass.pDepthStencilAttachment = &depthAttachmentRef;
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

		VkSubpassDependency dependency{};

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		std::array<VkAttachmentDescription, 3> attachments = { colorAttachments, depthAttachment, colorAttachmentResolve };
		VkRenderPassCreateInfo renderpassInfo{};
		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderpassInfo.pAttachments = attachments.data();
		renderpassInfo.dependencyCount = 1;
		renderpassInfo.pDependencies = &dependency;
		renderpassInfo.subpassCount = 1;
		renderpassInfo.pSubpasses = &subpass;

		VK_CHECK_RESULT(vkCreateRenderPass(m_renderer->getDevice(), &renderpassInfo, nullptr, &m_renderpass));


	}



	void triangle::CreateGraphicsPipelineLayout()
	{
		std::array<VkDescriptorSetLayout, 2> set_layout = { m_descriptorSetLayout.matrices, m_descriptorSetLayout.textures };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(set_layout.size());
		pipelineLayoutInfo.pSetLayouts = set_layout.data();

		VkPushConstantRange pushRange = initializer::pushConstantRange(VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), 0);

		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushRange;

		if (vkCreatePipelineLayout(m_renderer->getDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}
	}

	void triangle::CreateGraphicsPipeline()
	{

		auto vertexshader = readfile(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/triangle_vert.spv");
		auto fragmentshader = readfile(initializer::getProjectPath() + "/Vulkan_Renderer/Asset/Shaders/triangle_frag.spv");

		if (vertexshader.empty() && fragmentshader.empty()) { throw std::runtime_error("The file is empty or corrupted"); }


		VkShaderModule vertexModule = createShaderModule(vertexshader, m_renderer->getDevice());
		VkShaderModule fragmentModule = createShaderModule(fragmentshader, m_renderer->getDevice());


		VkPipelineShaderStageCreateInfo vertexshaderInfo{};
		vertexshaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertexshaderInfo.pName = "main";
		vertexshaderInfo.module = vertexModule;
		vertexshaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

		VkPipelineShaderStageCreateInfo fragmentshaderInfo{};
		fragmentshaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragmentshaderInfo.module = fragmentModule;
		fragmentshaderInfo.pName = "main";
		fragmentshaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		

		VkPipelineShaderStageCreateInfo shaderstage[] = { vertexshaderInfo, fragmentshaderInfo };

		const std::vector<VkVertexInputBindingDescription> vertexBinidingDescription{
			initializer::vertexInputBinding(0, sizeof(Vulkan_gltfModel::Vertex), VK_VERTEX_INPUT_RATE_VERTEX)
		};

		const std::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescription
		{
			initializer::vertexInputAttributeDescription(0, 0,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, pos)),
			initializer::vertexInputAttributeDescription(0, 1,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, normal)),
			initializer::vertexInputAttributeDescription(0, 2,VK_FORMAT_R32G32_SFLOAT,    offsetof(Vulkan_gltfModel::Vertex, uv)),
			initializer::vertexInputAttributeDescription(0, 3,VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, color)),
			initializer::vertexInputAttributeDescription(0, 4,VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(Vulkan_gltfModel::Vertex, tangent))

		};
		
		VkPipelineVertexInputStateCreateInfo vertexInputCI = initializer::vertexInputStateCreateInfo();
		vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescription.size());
		vertexInputCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexBinidingDescription.size());
		vertexInputCI.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();
		vertexInputCI.pVertexBindingDescriptions = vertexBinidingDescription.data();


		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI = initializer::inputAssemblySate(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FALSE, 0);
		VkViewport viewportCI = initializer::viewPort((float)m_renderer->m_width, (float)m_renderer->m_height, 0.0f, 1.0f);
		VkRect2D scissor = initializer::rect2D((uint32_t)m_renderer->m_width, (uint32_t)m_renderer->m_height, 0, 0);
		std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo dynamicstateCI = initializer::dynamicStateInfo(dynamicStates);
		VkPipelineViewportStateCreateInfo viewportstateCI = initializer::viewportStateInfo(1, 1, &viewportCI, &scissor);
		VkPipelineRasterizationStateCreateInfo rasterCI = initializer::razterizationState(VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE);
		VkPipelineMultisampleStateCreateInfo multisampleCI = initializer::MultiSampleStateInfo(m_renderer->maxSampleCount());
		VkPipelineColorBlendAttachmentState colorblendattachment = initializer::colorBlendAttachment(VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
		VkPipelineColorBlendStateCreateInfo colorblendstate = initializer::colorBlendInfo(1, &colorblendattachment);
		VkPipelineDepthStencilStateCreateInfo depthStencilstate = initializer::depthStencilInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);

		multisampleCI.sampleShadingEnable = VK_TRUE;
		multisampleCI.minSampleShading = 1.0f; // min fraction for sample shading; closer to one is smoother, This is done at expense of performance

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderstage;
		pipelineInfo.pVertexInputState = &vertexInputCI;
		pipelineInfo.pInputAssemblyState = &inputAssemblyCI;
		pipelineInfo.pViewportState = &viewportstateCI;
		pipelineInfo.pRasterizationState = &rasterCI;
		pipelineInfo.pMultisampleState = &multisampleCI;
		pipelineInfo.pColorBlendState = &colorblendstate;
		pipelineInfo.pDynamicState = &dynamicstateCI;
		pipelineInfo.pDepthStencilState = &depthStencilstate;

		pipelineInfo.layout = m_pipelineLayout;
		pipelineInfo.renderPass = m_renderpass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		//Object Pipeline
		cubemap.preparePipelines(m_pipelineLayout, m_renderpass);
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_renderer->getDevice(), m_renderer->getPipelineCache(), 1, &pipelineInfo, nullptr, &m_graphicsPipeline));


		//Wireframe Pipeline
		if (m_renderer->device_features.fillModeNonSolid)
		{
			rasterCI.polygonMode = VK_POLYGON_MODE_LINE;
			rasterCI.lineWidth = 1.0f;
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_renderer->getDevice(), m_renderer->getPipelineCache(), 1, &pipelineInfo, nullptr, &m_wireframePipeline))
		}


		rasterCI.polygonMode = VK_POLYGON_MODE_FILL;
		for (auto& material : sponza_model.g_materials)
		{
			struct MaterialSerializationData
			{
				VkBool32 alphaMask;
				float alphaMaskCutOff;
			}materialSpecializationData;

			materialSpecializationData.alphaMask = material.alphaMode == "MASK";
			materialSpecializationData.alphaMaskCutOff = material.alphaCutOff;

			//Constant fragment shader material parameters will be set using specialization constants
			std::vector<VkSpecializationMapEntry> specializationMapEntries = {
				initializer::specializationMapEntry(0, offsetof(MaterialSerializationData, alphaMask), sizeof(MaterialSerializationData::alphaMask)),
				initializer::specializationMapEntry(1, offsetof(MaterialSerializationData, alphaMaskCutOff), sizeof(MaterialSerializationData::alphaMaskCutOff))
			};

			VkSpecializationInfo specializationInfo = initializer::specializationInfo(specializationMapEntries, sizeof(materialSpecializationData), &materialSpecializationData);
			shaderstage[1].pSpecializationInfo = &specializationInfo;

			rasterCI.cullMode = material.doubleSided ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_renderer->getDevice(), m_renderer->getPipelineCache(), 1, &pipelineInfo, nullptr, &material.pipeline));
		}



		//Ui Pipeline

		//Shaders Pipeline

		vkDestroyShaderModule(m_renderer->getDevice(), vertexModule, nullptr);
		vkDestroyShaderModule(m_renderer->getDevice(), fragmentModule, nullptr);
	}

	void triangle::UpdateUniformBuffer(uint32_t currentImage)
	{
		static auto start_timer = std::chrono::high_resolution_clock::now();

		auto current_timer = std::chrono::high_resolution_clock::now();

		float time = std::chrono::duration<float, std::chrono::seconds::period>(current_timer - start_timer).count();

		UniformBufferObject ubo{};

		std::cout << "Camera Position" << camera.m_position.x << ", " << camera.m_position.y << ", " << camera.m_position.z << std::endl;
		std::cout << "Camera Rotation" << camera.m_rotation.x << ", " << camera.m_rotation.y << ", " << camera.m_rotation.z << std::endl;

		//ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(-45.0f) + time, glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view  = camera._matrices.view;
		ubo.proj  = camera._matrices.perspective;
		ubo.viewpos = camera.viewPos;

		memcpy(uniformBufferMapped, &ubo, sizeof(ubo));
	}

	void triangle::CreateDescriptorPool()
	{
		//If there is a problem try with multiplying with  MAX_FRAME_IN_FLIGHT
		std::vector<VkDescriptorPoolSize> poolSize =
		{
			initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1),
			initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(sponza_model.g_materials.size()) * 2),
			initializer::descriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
		};

		const uint32_t maxSets = static_cast<uint32_t>(sponza_model.g_materials.size()) + 2;

		VkDescriptorPoolCreateInfo descriptorpoolCI{};
		descriptorpoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorpoolCI.poolSizeCount = static_cast<uint32_t>(poolSize.size());
		descriptorpoolCI.pPoolSizes = poolSize.data();
		descriptorpoolCI.maxSets = maxSets;

		VK_CHECK_RESULT(vkCreateDescriptorPool(m_renderer->getDevice(), &descriptorpoolCI, nullptr, &m_descriptorPool));
	}

	void triangle::CreateDescriptorSets()
	{
		VkDescriptorSetAllocateInfo allocInfo = initializer::descriptorSetAllocInfo(&m_descriptorSetLayout.matrices, m_descriptorPool, 1);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_renderer->getDevice(), &allocInfo, &m_descriptorSet));

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffer;
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);
		VkWriteDescriptorSet writeDescriptorSet = initializer::writeDescriptorSet(m_descriptorSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &bufferInfo);
		vkUpdateDescriptorSets(m_renderer->getDevice(), 1, &writeDescriptorSet, 0, nullptr);


		for (auto& material : sponza_model.g_materials)
		{
			const VkDescriptorSetAllocateInfo alloc_Info = initializer::descriptorSetAllocInfo(&m_descriptorSetLayout.textures, m_descriptorPool, 1);
			VK_CHECK_RESULT(vkAllocateDescriptorSets(m_renderer->getDevice(), &alloc_Info, &material.descriptorSet));
			VkDescriptorImageInfo colorMap = sponza_model.getTextureDescriptor(material.baseColorTextureIndex);
			VkDescriptorImageInfo normalMap = sponza_model.getTextureDescriptor(material.normalTextureIndex);

			std::vector<VkWriteDescriptorSet> write_DescriptorSets =
			{
				initializer::writeDescriptorSet(material.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, &colorMap),
				initializer::writeDescriptorSet(material.descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, &normalMap)
			};
			vkUpdateDescriptorSets(m_renderer->getDevice(), static_cast<uint32_t>(write_DescriptorSets.size()), write_DescriptorSets.data(), 0, nullptr);
		}

		//SkyBox WriteInfo
		const VkDescriptorSetAllocateInfo skybox_allocInfo = initializer::descriptorSetAllocInfo(&m_descriptorSetLayout.textures, m_descriptorPool, 1);
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_renderer->getDevice(), &skybox_allocInfo, &cubemap.descriptorInfo.descriptorSet));
		
		VkWriteDescriptorSet skybox_writeDescriptor = initializer::writeDescriptorSet(cubemap.descriptorInfo.descriptorSet,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &cubemap.descriptorInfo.imageInfo);
		vkUpdateDescriptorSets(m_renderer->getDevice(), 1, &skybox_writeDescriptor, 0, nullptr);

		/*The End*/
	}

	void triangle::CreateFrameBuffer()
	{
		frameBuffers.resize(m_renderer->getSwapchain().swapchain_imageView.size());

		for (size_t i = 0; i < m_renderer->getSwapchain().swapchain_imageView.size(); i++)
		{
			

			std::array<VkImageView, 3> attachment = { colorImageView, depthImageView, m_renderer->getSwapchain().swapchain_imageView[i] };
			VkFramebufferCreateInfo frameCI{};
			frameCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameCI.renderPass = m_renderpass;
			frameCI.attachmentCount = static_cast<uint32_t>(attachment.size());
			frameCI.pAttachments = attachment.data();
			frameCI.height = m_renderer->getSwapchain().getSwapchainExtent().height;
			frameCI.width = m_renderer->getSwapchain().getSwapchainExtent().width;
			frameCI.layers = 1;


			VK_CHECK_RESULT(vkCreateFramebuffer(m_renderer->getDevice(), &frameCI, nullptr, &frameBuffers[i]));


		}
	}

	void triangle::AllocateCommandBuffer()
	{
		m_commandBuffer.resize(MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo cmdbufferInfo{};
		cmdbufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdbufferInfo.commandPool = m_renderer->getCommandPool();
		cmdbufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdbufferInfo.commandBufferCount = static_cast<uint32_t>( m_commandBuffer.size());


		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_renderer->getDevice(), &cmdbufferInfo, m_commandBuffer.data()));


	}

	void triangle::RecordCommandBuffer(VkCommandBuffer cmdBuffer, uint32_t imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = nullptr;

		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &beginInfo));

		VkRenderPassBeginInfo renderpassInfo{};

		renderpassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderpassInfo.renderPass = m_renderpass;
		renderpassInfo.framebuffer = frameBuffers[imageIndex];

		renderpassInfo.renderArea.offset = { 0, 0 };
		renderpassInfo.renderArea.extent = m_renderer->getSwapchain().getSwapchainExtent();

		std::array<VkClearValue, 2> clearValue{};
		clearValue[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValue[1].depthStencil = { 1.0f, 0 };

		renderpassInfo.clearValueCount = static_cast<uint32_t>(clearValue.size());
		renderpassInfo.pClearValues = clearValue.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderpassInfo, VK_SUBPASS_CONTENTS_INLINE);


		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, 
			m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
		
		


		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		viewport.width = static_cast<float>(m_renderer->getSwapchain().getSwapchainExtent().width);
		viewport.height = static_cast<float>(m_renderer->getSwapchain().getSwapchainExtent().height);

		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

		VkRect2D scissors{};
		scissors.offset = { 0,0 };
		scissors.extent = m_renderer->getSwapchain().getSwapchainExtent();

		vkCmdSetScissor(cmdBuffer, 0, 1, &scissors);

		cubemap.Draw_Skybox(cmdBuffer, m_pipelineLayout);

		m_wireframe = keys.wireframe;
		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_wireframe ? m_wireframePipeline : m_graphicsPipeline);
		sponza_model.draw(cmdBuffer, m_pipelineLayout);
		Gui_Overlay.Draw(cmdBuffer);


		vkCmdEndRenderPass(cmdBuffer);

		VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));
	}

	void triangle::CreatePrimitiveObject()
	{
		imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFence.resize(MAX_FRAMES_IN_FLIGHT);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_CHECK_RESULT(vkCreateSemaphore(m_renderer->getDevice(), &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]));
			VK_CHECK_RESULT(vkCreateSemaphore(m_renderer->getDevice(), &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]));
			VK_CHECK_RESULT(vkCreateFence(m_renderer->getDevice(), &fenceInfo, nullptr, &inFlightFence[i]));
		}

	}

	void triangle::draw()
	{

		{
			//std::cout << "WAITING FOR FENCES" << std::endl;
			vkWaitForFences(m_renderer->getDevice(), 1, &inFlightFence[currentFrame], VK_TRUE, UINT64_MAX);
		}

		VkResult res;
		uint32_t imageIndex;
		{
			//std::cout << "Aquiring next Image Process" << std::endl;
			res = vkAcquireNextImageKHR(m_renderer->getDevice(), m_renderer->getSwapchain().getSwapChain(), UINT64_MAX, imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
		}
		
		if (res == VK_ERROR_OUT_OF_DATE_KHR)
		{
			std::cout << "Out 0f Date" << std::endl;
			ReCreateSwapchain();
			return;

		}
		//Udating Uniform Buffer
		UpdateUniformBuffer(currentFrame);

		
		{
			//std::cout << "Resetting Fence Process" << std::endl;
			vkResetFences(m_renderer->getDevice(), 1, &inFlightFence[currentFrame]);
		}
		
		//Recording Command Buffer
		vkResetCommandBuffer(m_commandBuffer[currentFrame], 0);
		{
			//std::cout << "Recording Command Buffer Process" << std::endl;
			RecordCommandBuffer(m_commandBuffer[currentFrame], imageIndex);
		}

		//Submittting commandBuffer
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { imageAvailableSemaphore[currentFrame]};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandBuffer[currentFrame];

		VkSemaphore SignalSemaphore[] = { renderFinishedSemaphore[currentFrame]};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = SignalSemaphore;

		{
			//std::cout << "Queue Submiting Process" << std::endl;
			VK_CHECK_RESULT(vkQueueSubmit(m_renderer->getGraphicsQueue(), 1, &submitInfo, inFlightFence[currentFrame]));
			
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = SignalSemaphore;

		VkSwapchainKHR swapchains[] = { m_renderer->getSwapchain().getSwapChain() };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapchains;
		presentInfo.pImageIndices = &imageIndex;

		{
			//std::cout << "Presenting Process" << std::endl;
			res = vkQueuePresentKHR(m_renderer->getPresentQueue(), &presentInfo);
			if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR)
			{
				ReCreateSwapchain();
			}
			else if (res != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to Present Swapchain Images");
			}
		}

		camera.Update(0.02f);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void triangle::ReCreateSwapchain()
	{
		vkDeviceWaitIdle(m_renderer->getDevice());

		m_renderer->ReCreateSwapchain();

		//Destroying Resources
		vkDestroyImage(m_renderer->getDevice(), colorImage, nullptr);
		vkFreeMemory(m_renderer->getDevice(), colorImageMemory, nullptr);
		vkDestroyImageView(m_renderer->getDevice(), colorImageView, nullptr);

		//Depth resource
		vkDestroyImage(m_renderer->getDevice(), depthImage, nullptr);
		vkFreeMemory(m_renderer->getDevice(), depthImageMemory, nullptr);
		vkDestroyImageView(m_renderer->getDevice(), depthImageView, nullptr);

		for (auto& framebuffer : frameBuffers)
		{
			vkDestroyFramebuffer(m_renderer->getDevice(), framebuffer, nullptr);
		}

		//Recreating Frame Buffer Resources
		CreateDepthBufferResource();
		CreateColorBufferResource();
		CreateFrameBuffer();


	}

	void triangle::CreateColorBufferResource()
	{
		colorFormat = m_renderer->getSwapchain().getSwapchainImageFormat();
		uint32_t c_width = m_renderer->getSwapchain().getSwapchainExtent().width;
		uint32_t c_height = m_renderer->getSwapchain().getSwapchainExtent().height;

		createImage(c_width, c_height, colorFormat, m_renderer->maxSampleCount(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory, m_renderer);

		colorImageView = m_renderer->CreateimageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}

	void triangle::CreateDepthBufferResource()
	{
		depthFormat = m_renderer->findDepthFormat();
		uint32_t d_width = m_renderer->getSwapchain().getSwapchainExtent().width;
		uint32_t d_height = m_renderer->getSwapchain().getSwapchainExtent().height;
		createImage(d_width, d_height, depthFormat,m_renderer->maxSampleCount(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory, m_renderer);

		depthImageView = m_renderer->CreateimageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		m_renderer->transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
		
	}

	void triangle::Destroy()
	{

		Gui_Overlay.Destroy();
		//Color Resource
		vkDestroyImage(m_renderer->getDevice(), colorImage, nullptr);
		vkFreeMemory(m_renderer->getDevice(), colorImageMemory, nullptr);
		vkDestroyImageView(m_renderer->getDevice(), colorImageView, nullptr);

		//Depth resource
		vkDestroyImage(m_renderer->getDevice(), depthImage, nullptr);
		vkFreeMemory(m_renderer->getDevice(), depthImageMemory, nullptr);
		vkDestroyImageView(m_renderer->getDevice(), depthImageView, nullptr);
		
		vkDestroyBuffer(m_renderer->getDevice(), m_uniformBuffer, nullptr);
		vkFreeMemory(m_renderer->getDevice(), m_uniformBufferMemory, nullptr);
	

		vkDestroyDescriptorPool(m_renderer->getDevice(), m_descriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(m_renderer->getDevice(), m_descriptorSetLayout.matrices, nullptr);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyFence(m_renderer->getDevice(), inFlightFence[i], nullptr);
			vkDestroySemaphore(m_renderer->getDevice(), renderFinishedSemaphore[i], nullptr);
			vkDestroySemaphore(m_renderer->getDevice(), imageAvailableSemaphore[i], nullptr);
		}

		for (auto& framebuffer : frameBuffers)
		{
			vkDestroyFramebuffer(m_renderer->getDevice(), framebuffer, nullptr);
		}

		vkDestroyPipeline(m_renderer->getDevice(), m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_renderer->getDevice(), m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_renderer->getDevice(), m_renderpass, nullptr);
	}

}