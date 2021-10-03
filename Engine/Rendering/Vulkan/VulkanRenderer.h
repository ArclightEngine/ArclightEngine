#pragma once

#include <vulkan/vulkan.h>

#include <Arclight/Window/WindowContext.h>

#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Transform.h>

#include "VulkanMemory.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"

#include <optional>
#include <set>
#include <vector>
#include <unordered_map>

#define RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT 2
#define RENDERING_VULKANRENDERER_TEXTURE_SAMPLER_DESCRIPTOR 0

#define RENDERING_VULKANRENDERER_ENABLE_VALIDATION_LAYERS

namespace Arclight::Rendering {

class VulkanRenderer final : public Renderer {
    friend class VulkanPipeline;
    friend class VulkanTexture;

public:
    ~VulkanRenderer();

    int Initialize(WindowContext* windowContext);

    void Render();
    void WaitDeviceIdle() const; 

    void ResizeViewport(const Vector2i& pixelSize) override;

    // Get the default RenderPipeline
    RenderPipeline& DefaultPipeline();

    void BindPipeline(RenderPipeline::PipelineHandle pipeline) override;
    void BindTexture(Texture::TextureHandle texture) override;

    // Draw Primitives
    void Draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform = Matrix4());

    const std::string& GetName() const { return m_rendererName; }

    RenderPipeline::PipelineHandle
    CreatePipeline(const Shader& vertexShader, const Shader& fragmentShader,
                   const RenderPipeline::PipelineFixedConfig& config);
    void DestroyPipeline(RenderPipeline::PipelineHandle handle);

    Texture::TextureHandle AllocateTexture(const Vector2u& bounds);
    void UpdateTexture(Texture::TextureHandle texture, const void* data);
    void DestroyTexture(Texture::TextureHandle texture);

protected:
    // RAII Command Buffer wrapper
    class OneTimeCommandBuffer : NonCopyable {
    public:
        OneTimeCommandBuffer(VulkanRenderer& renderer, VkCommandPool pool);
        ~OneTimeCommandBuffer();

        inline VkCommandBuffer Buffer() { return m_buffer; }

    private:
        VulkanRenderer& m_renderer;
        VkCommandPool m_pool = VK_NULL_HANDLE;
        VkCommandBuffer m_buffer = VK_NULL_HANDLE;
    };

    inline VkDevice GetDevice() { return m_device; }
    inline VkRenderPass GetRenderPass() { return m_renderPass; }
    inline VkExtent2D GetScreenExtent() const { return m_swapExtent; }
    inline VmaAllocator Allocator() { return m_alloc; }

    inline OneTimeCommandBuffer CreateOneTimeCommandBuffer() {
        return OneTimeCommandBuffer(*this, m_commandPools[m_currentFrame]);
    }

    std::set<VulkanTexture*> m_textures;
    std::set<VulkanPipeline*> m_pipelines;
    RenderPipeline* m_defaultPipeline = nullptr;

    // For now all pipelines are required to have the same descriptor set layout
    // TODO: More configurable pipelines
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;

private:
    struct SwapChainInfo {
        VkSurfaceCapabilitiesKHR surfaceCapabilites;
        std::vector<VkSurfaceFormatKHR> surfaceFormats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    struct VertexBuffer {
        VmaAllocation allocation; // VmaAlloaction of vertex buffer
        VkBuffer buffer;          // VkBuffer object

        void* hostMapping; // Host memory mapping of the buffer
        uint32_t size;     // How many vertexes can fit in buffer
    };

    struct Frame {
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkSemaphore fence;

        VkCommandPool commandPool;
        VkCommandBuffer commandBuffer;
    };

    struct PipelineViewportInfo {
        VkExtent2D screenExtent;
        VkViewport viewport;
        VkRect2D scissor;
    } m_viewportInfo;

    struct DescriptorPool {
        std::vector<VkDescriptorSet> sets;
        VkDescriptorPool handle;

        // Index of the next set
        int nextSet = 0;
    };

    // Ready frame for drawing
    void BeginFrame();
    // Finish drawing frame
    void EndFrame();

    int LoadExtensions();

    int EnumerateGPUs();
    int SelectBestGPU();
    int GPUSuitability(VkPhysicalDevice dev);

    std::optional<uint32_t> GetGraphicsQueueFamily();
    SwapChainInfo GetSwapChainInfo();

    int CreateLogicalDevice();
    void CreateCommandPools();

    void CreateDescriptorSetLayout();
    DescriptorPool CreateDescriptorPool();

    // Begin recording command buffer
    void BeginCommandBuffer();
    // Finish recording command buffer
    void EndCommandBuffer();

    void BeginRenderPass();
    void EndRenderPass();

    VertexBuffer CreateVertexBuffer(uint32_t vertexCount);

    VkDescriptorSet AllocateDescriptorSet();

    const std::string m_rendererName = "Vulkan";

    WindowContext* m_windowContext = nullptr;
    VkSurfaceKHR m_surface;     // Vulkan surface
    VkInstance m_instance;      // Vulkan instance
    VkSwapchainKHR m_swapchain; // Vulkan swap chain

    VkRenderPass m_renderPass;

    VkSurfaceCapabilitiesKHR m_swapchainSurfaceCapabilities;
    VkPresentModeKHR m_swapchainPresentMode;
    VkFormat m_swapImageFormat = VK_FORMAT_UNDEFINED;
    VkColorSpaceKHR m_swapColourSpace;
    VkExtent2D m_swapExtent;

    Transform m_viewportTransform;

    VmaAllocator m_alloc; // VMA library allocator object

    // Each frame in the queue should have its own:
    // - Command pool
    // - Descriptor pool cache
    // - Descriptor set cache
    // - Buffer pool
    // See
    // https://github.com/KhronosGroup/Vulkan-Samples/blob/master/samples/performance/command_buffer_usage/command_buffer_usage_tutorial.md

    uint32_t m_imageIndex = 0;   // Vulkan image index of frame being rendered
    unsigned m_currentFrame = 0; // Our index of current frame being rendered
    VkSemaphore m_imageAvailableSemaphores[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VkSemaphore m_renderFinishedSemaphores[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VkFence m_frameFences[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VertexBuffer m_vertexBuffers[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];

    VulkanPipeline* m_lastPipelines[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VulkanPipeline* m_boundPipeline = nullptr;

    // Last used texture per pipeline
    // If the texture of the vertices being rendered is the same as the previous,
    // We do not need to update descriptor sets
    std::unordered_map<VulkanTexture*, VkDescriptorSet> m_textureDescriptorSets;
    VulkanTexture* m_lastTextures[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VulkanTexture* m_boundTexture = nullptr;

    DescriptorPool m_descriptorPools[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];

    std::vector<VkImage> m_images; // Swapchain image handles
    std::vector<VkImageView> m_imageViews;
    std::vector<VkFramebuffer> m_framebuffers; // Swapchain framebuffers

    VkDevice m_device = VK_NULL_HANDLE; // Logical device
    VkQueue m_graphicsQueue;            // Graphics queue
    uint32_t m_graphicsQueueFamily;

    std::vector<VkCommandPool> m_commandPools; // Command pool
    std::vector<VkCommandBuffer> m_commandBuffers;

    std::vector<const char*> m_vkExtensions; // List of Vulkan extension names

    std::vector<VkPhysicalDevice> m_GPUs; // List of Vulkan Physical Devices (GPUs)
    VkPhysicalDevice m_renderGPU;         // Our current GPU

    VkApplicationInfo m_vkAppInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Game",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "Game",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_2,
    };

    VkInstanceCreateInfo m_vkCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &m_vkAppInfo,
#ifdef RENDERING_VULKANRENDERER_ENABLE_VALIDATION_LAYERS
        .enabledLayerCount = 1,
#else
        .enabledLayerCount = 0,
#endif
        .ppEnabledLayerNames = validationLayers,
        .enabledExtensionCount = 0,
        .ppEnabledExtensionNames = nullptr,
    };

    const char* const validationLayers[1]{"VK_LAYER_KHRONOS_validation"};
};

} // namespace Arclight::Rendering
