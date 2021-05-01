#include "VulkanRenderer.h"

#include "ResourceManager.h"

#include <SDL2/SDL_vulkan.h>
#include <assert.h>

#include <stdexcept>

namespace Arclight::Rendering {

VulkanRenderer::~VulkanRenderer(){
	for(int i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++){
		vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_frameFences[i], nullptr);
	}

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	for(VkFramebuffer& fb : m_framebuffers){
		vkDestroyFramebuffer(m_device, fb, nullptr);
	}

	if(m_pipeline){
		delete m_pipeline;
	}
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	for(VkImageView v : m_imageViews){
		vkDestroyImageView(m_device, v, nullptr);
	}

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyDevice(m_device, nullptr);

	assert(s_rendererInstance == this);
	s_rendererInstance = nullptr;
}

int VulkanRenderer::Initialize(WindowContext* windowContext) {
	assert(!s_rendererInstance);

	m_windowContext = windowContext;

	SDL_Window* sdlWindow = m_windowContext->GetWindow();
	
	if(LoadExtensions()){
		return -2; // Failed to load extensions
	}

	// Create our Vulkan instance
	if(vkCreateInstance(&m_vkCreateInfo, nullptr, &m_instance) != VK_SUCCESS){
		fprintf(stderr, "VulkanRenderer::Initialize: Failed to create Vulkan instance!");
		return -1;
	}

	if(EnumerateGPUs()){
		return -4;
	} else if(!m_GPUs.size()){
		fprintf(stderr, "VulkanRenderer::Initialize: No available Vulkan devices!");
		return -5;
	}

	if(SelectBestGPU() || m_renderGPU == VK_NULL_HANDLE){
		fprintf(stderr, "VulkanRenderer::Initialize: No suitable Vulkan devices!");
		return -5;
	}

	{
		VkPhysicalDeviceProperties p;
		vkGetPhysicalDeviceProperties(m_renderGPU, &p);

		printf("Using GPU: %s\n", p.deviceName);
	}

	if(CreateLogicalDevice()){
		return -6;
	}

	// Create Vulkan surface with SDL
	if(!SDL_Vulkan_CreateSurface(sdlWindow, m_instance, &m_surface)){
		fprintf(stderr, "VulkanRenderer::Initialize: Failed to create SDL Vulkan surface!\n");
		return -3;
	}

	VkBool32 supportsSurface = false;
	if(vkGetPhysicalDeviceSurfaceSupportKHR(m_renderGPU, m_graphicsQueueFamily, m_surface, &supportsSurface) != VK_SUCCESS || !supportsSurface){
		fprintf(stderr, "VulkanRenderer::Initialize: GPU does not support Vulkan surface!\n");
		return -7;
	}

	SwapChainInfo scInfo = GetSwapChainInfo();
	assert(scInfo.presentModes.size() && scInfo.surfaceFormats.size());

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Surface present mode
	// It is required that VK_PRESENT_MODE_FIFO_KHR be supported
	// Essentially VSync, blocks the GPU when the image queue is full

	for(const VkPresentModeKHR& mode : scInfo.presentModes){
		if(mode == VK_PRESENT_MODE_MAILBOX_KHR){
			// VK_PRESENT_MODE_MAILBOX_KHR replaces existing images instead of blocking when the queue is full
			presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}
	}

	for(const VkSurfaceFormatKHR& sFormat : scInfo.surfaceFormats){
		if(sFormat.format == VK_FORMAT_B8G8R8A8_SRGB && sFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
			m_swapImageFormat = sFormat.format;
			m_swapColourSpace = sFormat.colorSpace;
		}
	}

	assert(m_swapImageFormat != VK_FORMAT_UNDEFINED); // We want ARGB 8bpp, sRGB colour space

	m_swapExtent = scInfo.surfaceCapabilites.currentExtent;
	if(scInfo.surfaceCapabilites.currentExtent.width == static_cast<uint32_t>(-1)){ // Swap chain size may not correspond to window size
		Vector2i windowSize = m_windowContext->GetWindowRenderSize();
		
		m_swapExtent.width = std::clamp(static_cast<uint32_t>(windowSize.x), scInfo.surfaceCapabilites.minImageExtent.width, scInfo.surfaceCapabilites.maxImageExtent.width);
		m_swapExtent.height = std::clamp(static_cast<uint32_t>(windowSize.y), scInfo.surfaceCapabilites.minImageExtent.height, scInfo.surfaceCapabilites.maxImageExtent.height);
	}

	uint32_t imageCount = scInfo.surfaceCapabilites.minImageCount + 1;
	if(scInfo.surfaceCapabilites.maxImageCount > 0){
		imageCount = std::clamp(imageCount, scInfo.surfaceCapabilites.minImageCount, scInfo.surfaceCapabilites.maxImageCount);
	}

	VkSwapchainCreateInfoKHR swapchainCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.pNext = nullptr,
		.flags = 0,
		.surface = m_surface,
		.minImageCount = imageCount,
		.imageFormat = m_swapImageFormat,
		.imageColorSpace = m_swapColourSpace,
		.imageExtent = m_swapExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, // Our graphics and present queues are the same
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = scInfo.surfaceCapabilites.currentTransform, // Use the current transform
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // We do not need a transparent window
		.presentMode = presentMode,
		.clipped = VK_TRUE, // Our window can be clipped
		.oldSwapchain = VK_NULL_HANDLE,
	};

	if(vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS){
		fprintf(stderr, "VulkanRenderer::Initialize: Failed to create swapchain!\n");
		return -8;
	}

	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
	m_images.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

	m_imageViews.resize(m_images.size());
	for(unsigned i = 0; i < m_images.size(); i++){
		VkImageViewCreateInfo ivCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.image = m_images[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_swapImageFormat,
			.components = {
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
			},
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1,
			},
		};

		if(vkCreateImageView(m_device, &ivCreateInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to create Vulkan image view!");
		}
	}

	VkAttachmentDescription colourAttachment = {
		.flags = 0,
		.format = m_swapImageFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT, // No multisampling
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE, // Clear screen on load
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Present to surface
	};

	VkAttachmentReference attachmentReference {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpass = {
		.flags = 0,
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = 0,
		.pInputAttachments = nullptr,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentReference,
		.pResolveAttachments = nullptr,
		.pDepthStencilAttachment = nullptr,
		.preserveAttachmentCount = 0,
		.pPreserveAttachments = nullptr,
	};

	VkSubpassDependency dep = {
		.srcSubpass = VK_SUBPASS_EXTERNAL,
		.dstSubpass = 0,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.srcAccessMask = 0,
		.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dependencyFlags = 0,
	};

	VkRenderPassCreateInfo renderPassCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.attachmentCount = 1,
		.pAttachments = &colourAttachment,
		.subpassCount = 1,
		.pSubpasses = &subpass,
		.dependencyCount = 1,
		.pDependencies = &dep,
	};

	if(vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::Initialize: Failed to create Vulkan render pass!");
		return -11;
	}

	m_framebuffers.resize(m_imageViews.size());
	for(unsigned i = 0; i < m_imageViews.size(); i++){
		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.renderPass = m_renderPass,
			.attachmentCount = 1,
			.pAttachments = &m_imageViews[i],
			.width = m_swapExtent.width,
			.height = m_swapExtent.height,
			.layers = 1,
		};

		if(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to create Vulkan framebuffer!");
			return -13;
		}
	}

	{
		std::vector<uint8_t> vertData;
		std::vector<uint8_t> fragData;

		ResourceManager::LoadResource("shaders/vert.spv", vertData);
		ResourceManager::LoadResource("shaders/frag.spv", fragData);

		VulkanShader vertShader(Shader::VertexShader, std::move(vertData));
		VulkanShader fragShader(Shader::FragmentShader, std::move(fragData));

		m_pipeline = new VulkanPipeline(*this, &vertShader, &fragShader);
	}

	CreateCommandPool();

	VkClearValue clearColour = {
		1.f / m_windowContext->backgroundColour.r,
		1.f / m_windowContext->backgroundColour.g,
		1.f / m_windowContext->backgroundColour.b,
		1.f / m_windowContext->backgroundColour.a,
	};

	for(unsigned i = 0; i < m_framebuffers.size(); i++){
		VkCommandBufferBeginInfo beginInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};

		if(vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to begin command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = nullptr,
			.renderPass = m_renderPass,
			.framebuffer = m_framebuffers[i],
			.renderArea = { // Render area
				{0, 0},
				m_swapExtent,
			},
			.clearValueCount = 1,
			.pClearValues = &clearColour, // Get our clear colour from the WindowContext
		};

		// The render pass commands will be embedded in the primary command buffer itself and no secondary command buffers will be executed.
		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->GetPipelineHandle());
		vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_commandBuffers[i]);

		if(vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to end command buffer!");
		}
	}

	VkSemaphoreCreateInfo semaphoreInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		.pNext = nullptr,
		.flags =  0,
	};
	
	VkFenceCreateInfo fenceInfo = {
		.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.pNext = nullptr,
		.flags =  VK_FENCE_CREATE_SIGNALED_BIT,
	};

	for(int i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++){
		if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to create semaphores!");
		}

		if(vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to create semaphores!");
		}
		
		if(vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameFences[i]) != VK_SUCCESS){
			throw std::runtime_error("VulkanRenderer::Initialize: Failed to create fences!");
		}
	}

	s_rendererInstance = this;
	return 0;
}

void VulkanRenderer::Draw(){
	vkWaitForFences(m_device, 1, &m_frameFences[currentFrame], VK_TRUE, UINT64_MAX);
	vkResetFences(m_device, 1, &m_frameFences[currentFrame]);

	uint32_t imageIndex = 0;
	int result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, m_imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
	assert(result == VK_SUCCESS);

	VkPipelineStageFlags waitStages[] = {
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
	};

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_imageAvailableSemaphores[currentFrame],
		.pWaitDstStageMask = waitStages,
		.commandBufferCount = 1,
		.pCommandBuffers = &m_commandBuffers[imageIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderFinishedSemaphores[currentFrame],
	};

	if(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFences[currentFrame]) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::Draw: Failed to submit command!");
	}

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.pNext = nullptr,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderFinishedSemaphores[currentFrame],
		.swapchainCount = 1,
		.pSwapchains = &m_swapchain,
		.pImageIndices = &imageIndex,
		.pResults = nullptr,
	};

	vkQueuePresentKHR(m_graphicsQueue, &presentInfo);
	currentFrame = (currentFrame + 1) % RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT;
}

void VulkanRenderer::WaitDeviceIdle() const {
	vkDeviceWaitIdle(m_device);
}

int VulkanRenderer::LoadExtensions(){
	SDL_Window* sdlWindow = m_windowContext->GetWindow();

	// Get the amount of Vulkan instance extensions from SDL
	unsigned int eCount;
	if(!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &eCount, nullptr)){
		fprintf(stderr, "VulkanRenderer::Initialize: Failed to get SDL Vulkan Extensions!\n");
		return -2;
	}

	// Get the actual extension names
	m_vkExtensions.resize(eCount);
	if(!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &eCount, m_vkExtensions.data())){
		fprintf(stderr, "VulkanRenderer::Initialize: Failed to get SDL Vulkan Extensions!\n");
		return -2;
	}

	m_vkCreateInfo.enabledExtensionCount = m_vkExtensions.size();
	m_vkCreateInfo.ppEnabledExtensionNames = m_vkExtensions.data();

	return 0;
}

int VulkanRenderer::EnumerateGPUs(){
	uint32_t deviceCount;
	if(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr) != VK_SUCCESS){ // Get Physical Device Count
		fprintf(stderr, "VulkanRenderer::EnumerateGPUs: Failed to enumerate Vulkan Physical Devices!\n");
		return -1;
	}

	m_GPUs.resize(deviceCount);
	if(vkEnumeratePhysicalDevices(m_instance, &deviceCount, m_GPUs.data())){
		fprintf(stderr, "VulkanRenderer::EnumerateGPUs: Failed to enumerate Vulkan Physical Devices!\n");
		return -1;
	}

	return 0;
}

int VulkanRenderer::SelectBestGPU(){
	assert(m_GPUs.size());

	int currentScore = 0;
	m_renderGPU = VK_NULL_HANDLE;

	for(auto& gpu : m_GPUs){
		int score = GPUSuitability(gpu);
		if(score > currentScore){
			currentScore = score;
			m_renderGPU = gpu;
		}
	}

	if(m_renderGPU == VK_NULL_HANDLE){
		return -1;
	}

	return 0;
}

int VulkanRenderer::GPUSuitability(VkPhysicalDevice dev){
	assert(dev);

	VkPhysicalDeviceFeatures features;
	vkGetPhysicalDeviceFeatures(dev, &features);

	if(!features.geometryShader) {
		return 0;
	}

	int score = 0;
	
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, extensions.data());

	for(const VkExtensionProperties& ext : extensions){
		if(!strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME)){ // Make sure it has swap chain support
			score = 10;
			break;
		}
	}

	if(score <= 0){ // If score is still zero there is no swapchain support
		return 0;
	}

	VkPhysicalDeviceProperties properties;
	vkGetPhysicalDeviceProperties(dev, &properties);
	
	if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
		score += 1000;
	} else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU){
		score += 100;
	}

	return score;
}

std::optional<uint32_t> VulkanRenderer::GetGraphicsQueueFamily(){
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_renderGPU, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_renderGPU, &queueFamilyCount, queueFamilies.data());

	for(uint32_t i = 0; i < queueFamilies.size(); i++){
		if(queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT){
			return std::optional<uint32_t>(i);
		}
	}

	return std::optional<uint32_t>();
}

VulkanRenderer::SwapChainInfo VulkanRenderer::GetSwapChainInfo(){
	assert(m_renderGPU != VK_NULL_HANDLE);

	SwapChainInfo info;

	if(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderGPU, m_surface, &info.surfaceCapabilites) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::GetSwapChainInfo: Failed to get surface capabilities!"); // This function does not return on failure, throw an exception
	}

	uint32_t formatCount;
	if(vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderGPU, m_surface, &formatCount, nullptr) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::GetSwapChainInfo: Failed to get surface formats!");
	}

	info.surfaceFormats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderGPU, m_surface, &formatCount, info.surfaceFormats.data());


	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderGPU, m_surface, &presentModeCount, nullptr);

	info.presentModes.resize(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderGPU, m_surface, &presentModeCount, info.presentModes.data());

	return info;
}

int VulkanRenderer::CreateLogicalDevice(){
	std::optional<uint32_t> gQueueFamily = GetGraphicsQueueFamily(); // Index of the first queue family found supporting graphics commands
	if(!gQueueFamily.has_value()){
		fprintf(stderr, "VulkanRenderer::CreateLogicalDevice: Failed to find graphics queue family!");
		return -1;
	}

	float queuePriority = 1.f; // Highest priority is 1, lowest is 0
	m_graphicsQueueFamily = gQueueFamily.value();

	VkDeviceQueueCreateInfo queueCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = gQueueFamily.value(),
		.queueCount = 1,
		.pQueuePriorities = &queuePriority,
	};

	VkPhysicalDeviceFeatures usedDeviceFeatures = {};

	const char* const enabledExtensions[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	}; // Currently we only enable swapchain extension
	
	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 1,
		.ppEnabledExtensionNames = enabledExtensions,
		.pEnabledFeatures = &usedDeviceFeatures,
	};

	if(vkCreateDevice(m_renderGPU, &createInfo, nullptr, &m_device) != VK_SUCCESS){
		fprintf(stderr, "VulkanRenderer::CreateLogicalDevice: Failed to create logical device!");
		return -1;
	}

	vkGetDeviceQueue(m_device, gQueueFamily.value(), 0, &m_graphicsQueue);

	return 0;
}

void VulkanRenderer::CreateCommandPool(){
	VkCommandPoolCreateInfo poolInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.queueFamilyIndex = m_graphicsQueueFamily,
	};

	if(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::CreateCommandPool: Failed to create a command pool!");
	}

	m_commandBuffers.resize(m_framebuffers.size());
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = m_commandPool,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size()),
	};

	if(vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::CreateCommandPool: Failed to create command buffers!");
	}
}

} // namespace Arclight::Rendering