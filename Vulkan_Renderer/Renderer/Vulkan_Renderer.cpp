#include "Vulkan_Renderer.h"
#include "rdpencomapi.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif


namespace tde
{
	Renderer::Renderer(Win* v_win)
	{
		this->m_win = v_win;
		if (enableValidationLayers)
		{
			set_upConsole();
		}

		//Window height && Width;
		if (GetClientRect(m_win->m_window, &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}

		//std::cout << m_width << ", " << m_height << std::endl;

		OutputDebugStringA("Renderer has been Initialized \n");
		Init();
	}

	Renderer::~Renderer()
	{
		this->Destroy();
	}

	void Renderer::Init()
	{
		Vulkan_Init();
	}

	void Renderer::Vulkan_Init()
	{
		//Vulkan Instance 
		Vulkan_Instance();
		//Vulkan Validation Layer
		if (enableValidationLayers)
			debug::setupDebugging(m_Instance);
		//Window Surface
		Window_Surface();
		//Vulkan Physical Device
		Vulkan_PhysicalDevice();
		//Vulkan Logical Device
		Vulkan_Device();
		
		//SwapChain Stuff
		prepare();

	}

	void Renderer::prepare()
	{
		Init_Swapchain();

		m_swapchain.CreateSwapchain();
		m_swapchain.CreateImageView();
		CreateCommandPool();

		CreatePipelineCache();
	}

	void Renderer::Init_Swapchain()
	{
		m_swapchain.connect(m_Instance, m_PhysicalDevice, m_Device, surface, m_width, m_height);
	}

	VkCommandBuffer Renderer::beginSingleCommand() const
	{
		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = m_commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_Device, &cmdAllocInfo, &cmdBuffer));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		return cmdBuffer;
	}

	void Renderer::endSingleCommand(VkCommandBuffer cmdBuffer) const
	{
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(m_Device, m_commandPool, 1, &cmdBuffer);
	}

	void Renderer::Vulkan_Instance()
	{
		bool not_sup = checkValidationLayerSupport();
		if (enableValidationLayers && !not_sup)
		{
			throw std::runtime_error("requested but not found the validation layer");
		}
		//Application Info
		VkApplicationInfo appInfo{};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.pEngineName = "Real_Time_Renderer_Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pApplicationName = "Real_Time_Application";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

		//Instance Info

		std::vector<const char*> instance_extension = { VK_KHR_SURFACE_EXTENSION_NAME };

		if (_WIN32)
			instance_extension.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
		//To-DO : Other PlatForms

		uint32_t extCount = 0;
		
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, nullptr);
		std::vector<VkExtensionProperties> extProperties(extCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extCount, extProperties.data());
		
		if (extProperties.size() > 0)
		{
			for (auto& extension : extProperties)
			{
				supportedInstanceExtension.push_back(extension.extensionName);
				OutputDebugStringA("Available Extension : ");
				OutputDebugStringA(extension.extensionName);
				OutputDebugStringA("\n");
			}
		}

		VkInstanceCreateInfo InstanceCI{};
		InstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		InstanceCI.pApplicationInfo = &appInfo;
		InstanceCI.pNext = nullptr;

		if (std::find(supportedInstanceExtension.begin(), supportedInstanceExtension.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != supportedInstanceExtension.end())
		{
			instance_extension.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			instance_extension.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
		}

		if (instance_extension.size() > 0)
		{
  			InstanceCI.enabledExtensionCount = static_cast<uint32_t>(instance_extension.size());
			InstanceCI.ppEnabledExtensionNames = instance_extension.data();
		}

		//Layer
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			InstanceCI.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			InstanceCI.ppEnabledLayerNames = validationLayers.data();

			debug::setupDebugingMessengerCreateInfo(debugCreateInfo);
			InstanceCI.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			InstanceCI.enabledLayerCount = 0;

			InstanceCI.pNext = nullptr;
		}
		VkResult res = vkCreateInstance(&InstanceCI, nullptr, &m_Instance);
		
		if (res != VK_SUCCESS)
			throw std::runtime_error("Can't able to load Vulkan Instance");
		else
			OutputDebugStringA("Successfully Created Instance");
		
	}

	bool Renderer::checkValidationLayerSupport() {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	void Renderer::Window_Surface()
	{
		VkWin32SurfaceCreateInfoKHR surfaceCI{};
		surfaceCI.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCI.hinstance = m_win->m_hInst;
		surfaceCI.hwnd = m_win->m_window;

		VkResult res = vkCreateWin32SurfaceKHR(m_Instance, &surfaceCI, nullptr, &surface);

		if (res != VK_SUCCESS)
			throw std::runtime_error("Cannot able to create Window Surface");
	}

	void Renderer::Vulkan_PhysicalDevice()
	{
		uint32_t device_count = 0;

		vkEnumeratePhysicalDevices(m_Instance, &device_count, nullptr);
		if (device_count == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(m_Instance, &device_count, devices.data());

		for (const auto& device : devices)
		{
			if (IsDeviceSuitable(device))
			{
				m_PhysicalDevice = device;
				m_msaaSamples = getMaxUsableSampleCount();
				break;
			}
		}
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);


		std::cout << deviceProperties.deviceName << std::endl;


		if (m_PhysicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find suitable Physical Device");
		}
	}

	void Renderer::Vulkan_Device()
	{
		// Query bindless extension, called Descriptor Indexing 
		VkPhysicalDeviceDescriptorIndexingFeatures indexing_feature{};
		indexing_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;
		indexing_feature.pNext = nullptr;

		VkPhysicalDeviceFeatures2 device_feature2{};
		device_feature2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
		device_feature2.pNext = &indexing_feature;

		vkGetPhysicalDeviceFeatures2(m_PhysicalDevice, &device_feature2);
		// For the feature to be correctly working, we need both the possibility to partially bind a descriptor,
		// as some entries in the bindless array will be empty, and SpirV runtime descriptors.

		bindlessSupport = indexing_feature.descriptorBindingPartiallyBound && indexing_feature.runtimeDescriptorArray;



		QueueFamiles indices = findQueueFamily(m_PhysicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphics_queue.value(), indices.present_queue.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		//VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		//Solves for Shading Aliasing
		device_features.sampleRateShading = VK_TRUE;
		device_features.fillModeNonSolid = VK_TRUE;
		createInfo.pEnabledFeatures = &device_features;


		createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		//createInfo.pNext = &device_feature2;

		if (bindlessSupport)
		{
			indexing_feature.descriptorBindingPartiallyBound = VK_TRUE;
			indexing_feature.runtimeDescriptorArray = VK_TRUE;

			device_feature2.pNext = &indexing_feature;
		}

		if (vkCreateDevice(m_PhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) {
			throw std::runtime_error("failed to create logical device!");
		}

		vkGetDeviceQueue(m_Device, indices.graphics_queue.value(), 0, &graphics_queue);
		vkGetDeviceQueue(m_Device, indices.present_queue.value(), 0, &present_queue);
	}

	uint32_t Renderer::findMemoryIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties) const 
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("Cannot able to fin Memory Index");
	}

	VkSampleCountFlagBits Renderer::getMaxUsableSampleCount()
	{
		VkPhysicalDeviceProperties props{};
		vkGetPhysicalDeviceProperties(m_PhysicalDevice, &props);

		VkSampleCountFlags count = props.limits.framebufferColorSampleCounts & props.limits.framebufferDepthSampleCounts;

		if (count & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
		if (count & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
		if (count & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
		if (count & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
		if (count & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
		if (count & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }

		return VK_SAMPLE_COUNT_1_BIT;
	}

	void Renderer::CreatePipelineCache()
	{
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VK_CHECK_RESULT(vkCreatePipelineCache(m_Device, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache));
	}
	
	void Renderer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldlayout, VkImageLayout newLayout) const
	{
		VkCommandBuffer cmdBuffer = beginSingleCommand();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldlayout;
		barrier.newLayout = newLayout;

		barrier.image = image;

		/*This is used to transfer queue ownership*/
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

			if (hasStencilComponent(format))
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}else
		{
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		else if (oldlayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}

		else if (oldlayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}

		else
		{
			throw std::invalid_argument("UnSupported Layout Transition");
		}

		vkCmdPipelineBarrier(cmdBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);



		endSingleCommand(cmdBuffer);
	}

	void Renderer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) const
	{
		VkCommandBuffer cmdBuffer = beginSingleCommand();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.mipLevel = 0;

		region.imageOffset = { 0,0 };
		region.imageExtent =
		{
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		endSingleCommand(cmdBuffer);
	}

	VkImageView Renderer::CreateimageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlag) const
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlag;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView m_image_view;
		VK_CHECK_RESULT(vkCreateImageView(m_Device, &viewInfo, nullptr, &m_image_view));

		return m_image_view;
	}

	void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const
	{
		VkCommandBufferAllocateInfo bufferInfo{};
		bufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferInfo.commandPool = m_commandPool;
		bufferInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferInfo.commandBufferCount = 1;

		VkCommandBuffer cmdBuffer;
		vkAllocateCommandBuffers(m_Device, &bufferInfo, &cmdBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(cmdBuffer, &beginInfo);

		VkBufferCopy copyRegion{};
		copyRegion.size = size;
		copyRegion.srcOffset = 0;
		copyRegion.dstOffset = 0;
		vkCmdCopyBuffer(cmdBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
		vkEndCommandBuffer(cmdBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		vkQueueSubmit(graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(graphics_queue);

		vkFreeCommandBuffers(m_Device, m_commandPool, 1, &cmdBuffer);

	}

	void Renderer::CreateCommandPool()
	{
		const Renderer::QueueFamiles queueFamilyIndices = findQueueFamily(m_PhysicalDevice);

		VkCommandPoolCreateInfo commandPoolInfo{};
		commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPoolInfo.queueFamilyIndex = queueFamilyIndices.graphics_queue.value();

		VK_CHECK_RESULT(vkCreateCommandPool(m_Device, &commandPoolInfo, nullptr, &m_commandPool));
	}

	bool Renderer::checkDeviceExtensionSupport(const VkPhysicalDevice device)
	{
		uint32_t extension_count;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> ext_properties(extension_count);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, ext_properties.data());

		std::set<std::string> required_extension(deviceExtensions.begin(), deviceExtensions.end());

		for (auto& extension : ext_properties)
		{
			required_extension.erase(extension.extensionName);
		}

		return required_extension.empty();
	}

	VkFormat Renderer::findSupportedFormt(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(m_PhysicalDevice, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw std::runtime_error("Failed to find supported format");
	}

	bool Renderer::IsDeviceSuitable(const VkPhysicalDevice physicalDevice)
	{
		QueueFamiles indices = findQueueFamily(physicalDevice);

		vkGetPhysicalDeviceFeatures(physicalDevice, &device_features);
		vkGetPhysicalDeviceProperties(physicalDevice, &device_properties);

		

		bool extensionSupported = checkDeviceExtensionSupport(physicalDevice);

		bool swapchain_adequate = false;
		if (extensionSupported)
		{
			Swapchain::SwapChainSupportDetails swapchain_details = m_swapchain.queryswapchain(physicalDevice, surface);
			swapchain_adequate = !swapchain_details.formats.empty() && !swapchain_details.presentModes.empty();
		}

		return  indices.isComplete() && extensionSupported && swapchain_adequate && device_features.samplerAnisotropy ;	
	}

	const void Renderer::ReCreateSwapchain() 
	{
		if (GetClientRect(m_win->m_window, &rect))
		{
			m_width = rect.right - rect.left;
			m_height = rect.bottom - rect.top;
		}

		m_swapchain.cleanup();

		Init_Swapchain();

		m_swapchain.CreateSwapchain();
		m_swapchain.CreateImageView();
	}


	Renderer::QueueFamiles Renderer::findQueueFamily (const VkPhysicalDevice physicalDevice) const 
	{
		uint32_t queue_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_count, nullptr);
		std::vector<VkQueueFamilyProperties> queueProperties(queue_count);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queue_count, queueProperties.data());

		int i = 0;
		QueueFamiles indices;

		for (const auto& qp : queueProperties)
		{

			if (qp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphics_queue = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.present_queue = i;
			}

			if (indices.isComplete())
				break;
		}

		return indices;
	}

	void Renderer::set_upConsole()
	{
		AllocConsole();
		AttachConsole(GetCurrentProcessId());
		FILE* stream;
		freopen_s(&stream, "CONIN$", "r", stdin);
		freopen_s(&stream, "CONOUT$", "w+", stdout);
		freopen_s(&stream, "CONOUT$", "w+", stderr);
		// Enable flags so we can color the output
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		DWORD dwMode = 0;
		GetConsoleMode(consoleHandle, &dwMode);
		dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		SetConsoleMode(consoleHandle, dwMode);
		SetConsoleTitle(TEXT("VULKAN CONSOLE"));

	}

	void Renderer::Destroy()
	{
		OutputDebugStringA("Renderer has been Destroyed \n");
		vkDestroyPipelineCache(m_Device, m_pipelineCache, nullptr);
		vkDestroyCommandPool(m_Device, m_commandPool, nullptr);
		m_swapchain.cleanup();
		vkDestroyDevice(m_Device, nullptr);
		debug::freeDebugCallback(m_Instance);
		vkDestroySurfaceKHR(m_Instance, surface, nullptr);
		vkDestroyInstance(m_Instance, nullptr);
	}
}