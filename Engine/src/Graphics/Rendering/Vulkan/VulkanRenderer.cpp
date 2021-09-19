#include "VulkanRenderer.h"
#include "VulkanPrivate.h"

#include "VulkanMemory.h"

#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>

#include <SDL2/SDL_vulkan.h>
#include <assert.h>

#include <stdexcept>

namespace Arclight::Rendering {

#include "DefaultShaderBytecode.h"

VulkanRenderer::~VulkanRenderer() {
    EndRenderPass();

    vkQueueWaitIdle(m_graphicsQueue);

    vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

    for (int i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++) {
        vkFreeCommandBuffers(m_device, m_commandPools[i], 1, &m_commandBuffers[i]);
        vkDestroyCommandPool(m_device, m_commandPools[i], nullptr);

        vmaDestroyBuffer(m_alloc, m_vertexBuffers[i].buffer, m_vertexBuffers[i].allocation);

        vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(m_device, m_frameFences[i], nullptr);
    }

    for (VulkanTexture* texture : m_textures) {
        delete texture;
    }

    vmaDestroyAllocator(m_alloc);

    for (VkFramebuffer& fb : m_framebuffers) {
        vkDestroyFramebuffer(m_device, fb, nullptr);
    }

    for (VulkanPipeline* pipeline : m_pipelines) {
        delete pipeline;
    }
    vkDestroyRenderPass(m_device, m_renderPass, nullptr);

    if (m_defaultPipeline) {
        delete m_defaultPipeline;
    }

    for (VkImageView v : m_imageViews) {
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

    if (LoadExtensions()) {
        return -2; // Failed to load extensions
    }

    m_vkCreateInfo.enabledLayerCount = 0;

    // Create our Vulkan instance
    if (vkCreateInstance(&m_vkCreateInfo, nullptr, &m_instance) != VK_SUCCESS) {
        Logger::Error("VulkanRenderer::Initialize: Failed to create Vulkan instance!");
        return -1;
    }

    if (EnumerateGPUs()) {
        return -4;
    } else if (!m_GPUs.size()) {
        Logger::Error("VulkanRenderer::Initialize: No available Vulkan devices!");
        return -5;
    }

    if (SelectBestGPU() || m_renderGPU == VK_NULL_HANDLE) {
        Logger::Error("VulkanRenderer::Initialize: No suitable Vulkan devices!");
        return -5;
    }

    {
        VkPhysicalDeviceProperties p;
        vkGetPhysicalDeviceProperties(m_renderGPU, &p);

        Logger::Debug("Using GPU: ", p.deviceName);
    }

    if (CreateLogicalDevice()) {
        return -6;
    }

    // Create Vulkan surface with SDL
    if (!SDL_Vulkan_CreateSurface(sdlWindow, m_instance, &m_surface)) {
        Logger::Error("VulkanRenderer::Initialize: Failed to create SDL Vulkan surface!");
        return -3;
    }

    VkBool32 supportsSurface = false;
    if (vkGetPhysicalDeviceSurfaceSupportKHR(m_renderGPU, m_graphicsQueueFamily, m_surface,
                                             &supportsSurface) != VK_SUCCESS ||
        !supportsSurface) {
        Logger::Error("VulkanRenderer::Initialize: GPU does not support Vulkan surface!");
        return -7;
    }

    SwapChainInfo scInfo = GetSwapChainInfo();
    assert(scInfo.presentModes.size() && scInfo.surfaceFormats.size());

    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Surface present mode
    // It is required that VK_PRESENT_MODE_FIFO_KHR be supported
    // Essentially VSync, blocks the GPU when the image queue is full

    for (const VkPresentModeKHR& mode : scInfo.presentModes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            // VK_PRESENT_MODE_MAILBOX_KHR replaces existing images instead of blocking when the
            // queue is full
            presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    for (const VkSurfaceFormatKHR& sFormat : scInfo.surfaceFormats) {
        if (sFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            sFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            m_swapImageFormat = sFormat.format;
            m_swapColourSpace = sFormat.colorSpace;
        }
    }

    assert(m_swapImageFormat != VK_FORMAT_UNDEFINED); // We want ARGB 8bpp, sRGB colour space

    m_swapExtent = scInfo.surfaceCapabilites.currentExtent;
    if (scInfo.surfaceCapabilites.currentExtent.width ==
        static_cast<uint32_t>(-1)) { // Swap chain size may not correspond to window size
        Vector2i windowSize = m_windowContext->GetWindowRenderSize();

        m_swapExtent.width = std::clamp(static_cast<uint32_t>(windowSize.x),
                                        scInfo.surfaceCapabilites.minImageExtent.width,
                                        scInfo.surfaceCapabilites.maxImageExtent.width);
        m_swapExtent.height = std::clamp(static_cast<uint32_t>(windowSize.y),
                                         scInfo.surfaceCapabilites.minImageExtent.height,
                                         scInfo.surfaceCapabilites.maxImageExtent.height);
    }

    uint32_t imageCount = scInfo.surfaceCapabilites.minImageCount + 1;
    if (scInfo.surfaceCapabilites.maxImageCount > 0) {
        imageCount = std::clamp(imageCount, scInfo.surfaceCapabilites.minImageCount,
                                scInfo.surfaceCapabilites.maxImageCount);
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
        .imageSharingMode =
            VK_SHARING_MODE_EXCLUSIVE, // Our graphics and present queues are the same
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = scInfo.surfaceCapabilites.currentTransform, // Use the current transform
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR, // We do not need a transparent window
        .presentMode = presentMode,
        .clipped = VK_TRUE, // Our window can be clipped
        .oldSwapchain = VK_NULL_HANDLE,
    };

    if (vkCreateSwapchainKHR(m_device, &swapchainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        Logger::Error("VulkanRenderer::Initialize: Failed to create swapchain!\n");
        return -8;
    }

    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device, m_swapchain, &imageCount, m_images.data());

    m_imageViews.resize(m_images.size());
    for (unsigned i = 0; i < m_images.size(); i++) {
        VkImageViewCreateInfo ivCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = m_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = m_swapImageFormat,
            .components =
                {
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                    VK_COMPONENT_SWIZZLE_IDENTITY,
                },
            .subresourceRange =
                {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        };

        vkCheck(vkCreateImageView(m_device, &ivCreateInfo, nullptr, &m_imageViews[i]));
    }

    VkAttachmentDescription colourAttachment = {
        .flags = 0,
        .format = m_swapImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,      // No multisampling
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR, // Clear screen on load
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, // Present to surface
    };

    VkAttachmentReference attachmentReference{
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

    if (vkCreateRenderPass(m_device, &renderPassCreateInfo, nullptr, &m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanRenderer::Initialize: Failed to create Vulkan render pass!");
        return -11;
    }

    m_framebuffers.resize(m_imageViews.size());
    for (unsigned i = 0; i < m_imageViews.size(); i++) {
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

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error(
                "VulkanRenderer::Initialize: Failed to create Vulkan framebuffer!");
            return -13;
        }
    }

    m_viewportTransform =
        Transform({-1, -1}, {2.f / m_swapExtent.width, 2.f / m_swapExtent.height});
    s_rendererInstance = this;

    CreateDescriptorSetLayout();
    CreateDescriptorPool();

    CreateCommandPools();

    VkSemaphoreCreateInfo semaphoreInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    VmaAllocatorCreateInfo allocatorInfo = {
        .flags = 0,
        .physicalDevice = m_renderGPU,
        .device = m_device,
        .preferredLargeHeapBlockSize = 0, // Use default.
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .frameInUseCount = 0,
        .pHeapSizeLimit = nullptr, // Default
        .pVulkanFunctions = nullptr,
        .pRecordSettings = nullptr,
        .instance = m_instance,
        .vulkanApiVersion = VK_API_VERSION_1_2,
    };

    vkCheck(vmaCreateAllocator(&allocatorInfo, &m_alloc));

    {
        Shader vertShader(Shader::VertexShader, defaultVertexShaderData);
        Shader fragShader(Shader::FragmentShader, defaultFragmentShaderData);

        m_defaultPipeline = new RenderPipeline(vertShader, fragShader);
    }

    for (int i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++) {
        vkCheck(
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]));

        vkCheck(
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]));

        vkCheck(vkCreateFence(m_device, &fenceInfo, nullptr, &m_frameFences[i]));

        m_vertexBuffers[i] =
            CreateVertexBuffer(4); // Each frame gets a default vertex buffer of 4 vertices
        m_lastTextures[i] = nullptr;
    }

    BeginFrame();
    BeginRenderPass();
    return 0;
}

RenderPipeline::PipelineHandle
VulkanRenderer::CreatePipeline(const Shader& vertexShader, const Shader& fragmentShader,
                               const RenderPipeline::PipelineFixedConfig& config) {
    VulkanPipeline* pipeline = new VulkanPipeline(*this, vertexShader, fragmentShader, config);

    m_pipelines.insert(pipeline);

    return pipeline;
}

void VulkanRenderer::DestroyPipeline(RenderPipeline::PipelineHandle handle) {
    size_t erased = m_pipelines.erase(reinterpret_cast<VulkanPipeline*>(handle));
    assert(erased ==
           1); // Erase returns the amount of pipelines erased, ensure that this is exactly 1

    delete reinterpret_cast<VulkanPipeline*>(handle);
}

Texture::TextureHandle VulkanRenderer::AllocateTexture(const Vector2u& bounds) {
    VulkanTexture* texture = new VulkanTexture(*this, bounds);

    m_textures.insert(texture);

    return texture;
}

void VulkanRenderer::UpdateTexture(Texture::TextureHandle texture, const void* data) {
    assert(m_textures.contains(reinterpret_cast<VulkanTexture*>(texture)));

    VulkanTexture* vkTex = reinterpret_cast<VulkanTexture*>(texture);

    vkTex->UpdateTextureBuffer(data);
    vkTex->UpdateTextureImage();
}

void VulkanRenderer::DestroyTexture(Texture::TextureHandle texture) {
    size_t erased = m_textures.erase(reinterpret_cast<VulkanTexture*>(texture));
    assert(erased ==
           1); // Erase returns the amount of pipelines erased, ensure that this is exactly 1

    for (unsigned i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++) {
        if (m_lastTextures[i] == texture) {
            m_lastTextures[i] = nullptr; // Invalidate m_lastTexture if relevant
        }
    }

    delete reinterpret_cast<VulkanTexture*>(texture);
}

void VulkanRenderer::Render() {
    Renderer::Render();

    EndRenderPass();
    EndFrame();

    BeginFrame();
    BeginRenderPass();
}

void VulkanRenderer::WaitDeviceIdle() const { vkDeviceWaitIdle(m_device); }

RenderPipeline& VulkanRenderer::DefaultPipeline() { return *m_defaultPipeline; }

void VulkanRenderer::Draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform,
                          Texture::TextureHandle texture, RenderPipeline& pipeline) {
    if (vertexCount > m_vertexBuffers[m_currentFrame].size) {
        throw std::runtime_error("VulkanRenderer::Draw: vertexCount too large!");
    }

    if (texture && texture != m_lastTextures[m_currentFrame]) {
        m_lastTextures[m_currentFrame] = reinterpret_cast<VulkanTexture*>(texture);

        VkWriteDescriptorSet descriptorWrite = {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .pNext = nullptr,
            .dstSet = m_descriptorSets[m_currentFrame],
            .dstBinding = RENDERING_VULKANRENDERER_TEXTURE_SAMPLER_DESCRIPTOR,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &reinterpret_cast<VulkanTexture*>(texture)->DescriptorImageInfo(),
            .pBufferInfo = nullptr,
            .pTexelBufferView = nullptr,
        };

        vkUpdateDescriptorSets(m_device, 1, &descriptorWrite, 0,
                               nullptr); // Update sampler descriptor
    }

    // Only rebind the pipeline and descriptor sets
    // and update the viewport transform
    // when the pipeline has changed.
    VulkanPipeline* pipelineObj = reinterpret_cast<VulkanPipeline*>(m_defaultPipeline->Handle());
    if (pipelineObj != m_lastPipelines[m_currentFrame]) {
        m_lastPipelines[m_currentFrame] = pipelineObj;

        vkCmdBindPipeline(m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
                          pipelineObj->GetPipelineHandle());
        vkCmdBindDescriptorSets(
            m_commandBuffers[m_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS,
            reinterpret_cast<VulkanPipeline*>(pipeline.Handle())->PipelineLayout(), 0, 1,
            &m_descriptorSets[m_currentFrame], 0, nullptr);

        reinterpret_cast<VulkanPipeline*>(pipeline.Handle())
            ->UpdatePushConstant(m_commandBuffers[m_currentFrame],
                                 offsetof(VulkanPipeline::PushConstant2DTransform, viewport),
                                 16 * sizeof(float) /* 4x4 float matrix */,
                                 m_viewportTransform.Matrix().Matrix());
    }

    VertexBuffer& vertexBuffer = m_vertexBuffers[m_currentFrame];
    memcpy(vertexBuffer.hostMapping, vertices, sizeof(Vertex) * vertexCount);

    VkBuffer vertexBuffers[] = {vertexBuffer.buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(m_commandBuffers[m_currentFrame], 0, 1, vertexBuffers, offsets);

    reinterpret_cast<VulkanPipeline*>(pipeline.Handle())
        ->UpdatePushConstant(m_commandBuffers[m_currentFrame],
                             offsetof(VulkanPipeline::PushConstant2DTransform, transform),
                             16 * sizeof(float) /* 4x4 float matrix */, transform.Matrix());

    vkCmdDraw(m_commandBuffers[m_currentFrame], vertexCount, 1, 0, 0);
}

VulkanRenderer::OneTimeCommandBuffer::OneTimeCommandBuffer(VulkanRenderer& renderer,
                                                           VkCommandPool pool)
    : m_renderer(renderer), m_pool(pool) {
    VkCommandBufferAllocateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    vkCheck(vkAllocateCommandBuffers(m_renderer.m_device, &bufferInfo, &m_buffer));

    VkCommandBufferBeginInfo bufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    vkCheck(vkBeginCommandBuffer(m_buffer, &bufferBeginInfo));
}

VulkanRenderer::OneTimeCommandBuffer::~OneTimeCommandBuffer() {
    if (m_buffer == VK_NULL_HANDLE) {
        return;
    }

    vkCheck(vkEndCommandBuffer(m_buffer));

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = 0,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_buffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr,
    };

    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
    };

    VkFence fence;
    vkCheck(vkCreateFence(m_renderer.m_device, &fenceInfo, nullptr, &fence)); // Create a fence

    vkCheck(
        vkQueueSubmit(m_renderer.m_graphicsQueue, 1, &submitInfo, fence)); // Submit to the queue

    vkWaitForFences(m_renderer.m_device, 1, &fence, VK_TRUE,
                    UINT64_MAX); // Make sure we are finished

    vkDestroyFence(m_renderer.m_device, fence, nullptr);
    vkFreeCommandBuffers(m_renderer.m_device, m_pool, 1, &m_buffer);
}

void VulkanRenderer::BeginFrame() {
    VkResult result = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX,
                                            m_imageAvailableSemaphores[m_currentFrame],
                                            VK_NULL_HANDLE, &m_imageIndex);
    assert(result == VK_SUCCESS);

    vkWaitForFences(m_device, 1, &m_frameFences[m_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(m_device, 1, &m_frameFences[m_currentFrame]);

    vkCheck(vkResetCommandPool(m_device, m_commandPools[m_currentFrame],
                               0)); // Apparently resetting the whole command pool is faster
    BeginCommandBuffer();

    m_lastPipelines[m_currentFrame] = 0;
}

void VulkanRenderer::EndFrame() {
    EndCommandBuffer();

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_commandBuffers[m_currentFrame],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame],
    };

    vkCheck(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_frameFences[m_currentFrame]));

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame],
        .swapchainCount = 1,
        .pSwapchains = &m_swapchain,
        .pImageIndices = &m_imageIndex,
        .pResults = nullptr,
    };

    vkQueuePresentKHR(m_graphicsQueue, &presentInfo);

    m_currentFrame = (m_currentFrame + 1) % RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT;
}

int VulkanRenderer::LoadExtensions() {
    SDL_Window* sdlWindow = m_windowContext->GetWindow();

    // Get the amount of Vulkan instance extensions from SDL
    unsigned int eCount;
    if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &eCount, nullptr)) {
        Logger::Error("VulkanRenderer::Initialize: Failed to get SDL Vulkan Extensions!\n");
        return -2;
    }

    // Get the actual extension names
    m_vkExtensions.resize(eCount);
    if (!SDL_Vulkan_GetInstanceExtensions(sdlWindow, &eCount, m_vkExtensions.data())) {
        Logger::Error("VulkanRenderer::Initialize: Failed to get SDL Vulkan Extensions!\n");
        return -2;
    }

    m_vkCreateInfo.enabledExtensionCount = m_vkExtensions.size();
    m_vkCreateInfo.ppEnabledExtensionNames = m_vkExtensions.data();

    return 0;
}

int VulkanRenderer::EnumerateGPUs() {
    uint32_t deviceCount;
    if (vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr) !=
        VK_SUCCESS) { // Get Physical Device Count
        Logger::Error(
            "VulkanRenderer::EnumerateGPUs: Failed to enumerate Vulkan Physical Devices!\n");
        return -1;
    }

    m_GPUs.resize(deviceCount);
    if (vkEnumeratePhysicalDevices(m_instance, &deviceCount, m_GPUs.data())) {
        Logger::Error(
            "VulkanRenderer::EnumerateGPUs: Failed to enumerate Vulkan Physical Devices!\n");
        return -1;
    }

    return 0;
}

int VulkanRenderer::SelectBestGPU() {
    assert(m_GPUs.size());

    int currentScore = 0;
    m_renderGPU = VK_NULL_HANDLE;

    for (auto& gpu : m_GPUs) {
        int score = GPUSuitability(gpu);
        if (score > currentScore) {
            currentScore = score;
            m_renderGPU = gpu;
        }
    }

    if (m_renderGPU == VK_NULL_HANDLE) {
        return -1;
    }

    return 0;
}

int VulkanRenderer::GPUSuitability(VkPhysicalDevice dev) {
    assert(dev);

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(dev, &features);

    if (!features.geometryShader) {
        return 0;
    }

    int score = 0;

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, extensions.data());

    for (const VkExtensionProperties& ext : extensions) {
        if (!strcmp(ext.extensionName,
                    VK_KHR_SWAPCHAIN_EXTENSION_NAME)) { // Make sure it has swap chain support
            score = 10;
            break;
        }
    }

    if (score <= 0) { // If score is still zero there is no swapchain support
        return 0;
    }

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(dev, &properties);

    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    } else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
        score += 100;
    }

    return score;
}

std::optional<uint32_t> VulkanRenderer::GetGraphicsQueueFamily() {
    uint32_t queueFamilyCount;
    vkGetPhysicalDeviceQueueFamilyProperties(m_renderGPU, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_renderGPU, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            return std::optional<uint32_t>(i);
        }
    }

    return std::optional<uint32_t>();
}

VulkanRenderer::SwapChainInfo VulkanRenderer::GetSwapChainInfo() {
    assert(m_renderGPU != VK_NULL_HANDLE);

    SwapChainInfo info;

    if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_renderGPU, m_surface,
                                                  &info.surfaceCapabilites) != VK_SUCCESS) {
        throw std::runtime_error("VulkanRenderer::GetSwapChainInfo: Failed to get surface "
                                 "capabilities!"); // This function does not return on failure,
                                                   // throw an exception
    }

    uint32_t formatCount;
    if (vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderGPU, m_surface, &formatCount, nullptr) !=
        VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanRenderer::GetSwapChainInfo: Failed to get surface formats!");
    }

    info.surfaceFormats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_renderGPU, m_surface, &formatCount,
                                         info.surfaceFormats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderGPU, m_surface, &presentModeCount, nullptr);

    info.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_renderGPU, m_surface, &presentModeCount,
                                              info.presentModes.data());

    return info;
}

int VulkanRenderer::CreateLogicalDevice() {
    std::optional<uint32_t> gQueueFamily =
        GetGraphicsQueueFamily(); // Index of the first queue family found supporting graphics
                                  // commands
    if (!gQueueFamily.has_value()) {
        Logger::Error("VulkanRenderer::CreateLogicalDevice: Failed to find graphics queue family!");
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

    if (vkCreateDevice(m_renderGPU, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
        Logger::Error("VulkanRenderer::CreateLogicalDevice: Failed to create logical device!");
        return -1;
    }

    vkGetDeviceQueue(m_device, gQueueFamily.value(), 0, &m_graphicsQueue);

    return 0;
}

void VulkanRenderer::CreateCommandPools() {
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueFamilyIndex = m_graphicsQueueFamily,
    };

    VkCommandBufferAllocateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = VK_NULL_HANDLE,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    m_commandBuffers.resize(m_framebuffers.size());
    m_commandPools.resize(m_framebuffers.size());

    for (unsigned i = 0; i < RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT; i++) {
        vkCheck(vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPools[i]));

        bufferInfo.commandPool = m_commandPools[i];
        vkCheck(vkAllocateCommandBuffers(m_device, &bufferInfo, &m_commandBuffers[i]));
    }
}

void VulkanRenderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding samplerLayoutBinding = {
        .binding = RENDERING_VULKANRENDERER_TEXTURE_SAMPLER_DESCRIPTOR,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, // Combined image sampler
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT, // Use in fragment shader
        .pImmutableSamplers = nullptr,
    };

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = 1,
        .pBindings = &samplerLayoutBinding,
    };

    vkCheck(vkCreateDescriptorSetLayout(m_device, &descriptorSetLayoutCreateInfo, nullptr,
                                        &m_descriptorSetLayout));
}

void VulkanRenderer::CreateDescriptorPool() {
    VkDescriptorPoolSize samplerSize = {
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT, // One set for each frame
    };

    VkDescriptorPoolCreateInfo poolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT,
        .poolSizeCount = 1,
        .pPoolSizes = &samplerSize,
    };

    // TODO: This is VERY wasteful, we need a layout in an array for each set, find a better way to
    // do this
    std::vector<VkDescriptorSetLayout> layouts(RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT,
                                               m_descriptorSetLayout);
    vkCheck(vkCreateDescriptorPool(m_device, &poolCreateInfo, nullptr, &m_descriptorPool));

    VkDescriptorSetAllocateInfo setAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = m_descriptorPool,
        .descriptorSetCount = RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT,
        .pSetLayouts = layouts.data(),
    };
    vkCheck(vkAllocateDescriptorSets(m_device, &setAllocInfo, m_descriptorSets));
}

void VulkanRenderer::BeginCommandBuffer() {
    VkCommandBufferBeginInfo beginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr,
    };

    vkCheck(vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo));
}

void VulkanRenderer::EndCommandBuffer() {
    vkCheck(vkEndCommandBuffer(m_commandBuffers[m_currentFrame]));
}

void VulkanRenderer::BeginRenderPass() {
    VkClearValue clear = {{{
        1.0f - 1.0f / m_windowContext->backgroundColour.r,
        1.0f - 1.0f / m_windowContext->backgroundColour.g,
        1.0f - 1.0f / m_windowContext->backgroundColour.b,
        1.0f - 1.0f / m_windowContext->backgroundColour.a,
    }}};

    VkRenderPassBeginInfo renderPassInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = m_renderPass,
        .framebuffer = m_framebuffers[m_imageIndex],
        .renderArea =
            {
                // Render area
                {0, 0},
                m_swapExtent,
            },
        .clearValueCount = 1,
        .pClearValues = &clear,
    };

    // The render pass commands will be embedded in the primary command buffer itself and no
    // secondary command buffers will be executed.
    vkCmdBeginRenderPass(m_commandBuffers[m_currentFrame], &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanRenderer::EndRenderPass() { vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]); }

VulkanRenderer::VertexBuffer VulkanRenderer::CreateVertexBuffer(uint32_t vertexCount) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = vertexCount * sizeof(Vertex),
        .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VmaAllocationCreateInfo allocCreateInfo = {
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_CPU_ONLY,
        .requiredFlags = 0,
        .preferredFlags = 0,
        .memoryTypeBits = 0,
        .pool = 0,
        .pUserData = nullptr,
        .priority = 0.0f,
    };

    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocInfo = {};

    vkCheck(
        vmaCreateBuffer(m_alloc, &bufferInfo, &allocCreateInfo, &buffer, &allocation, &allocInfo));

    VertexBuffer vertexBuffer = {
        .allocation = allocation,
        .buffer = buffer,
        .hostMapping = allocInfo.pMappedData,
        .size = vertexCount,
    };

    return vertexBuffer;
}

} // namespace Arclight::Rendering
