#pragma once

#include <vulkan/vulkan.h>

#include <Arclight/Window/WindowContext.h>

#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Transform.h>

#include "VulkanMemory.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"

#include <cassert>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

#define RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT 2
#define RENDERING_VULKANRENDERER_TEXTURE_SAMPLER_DESCRIPTOR 0

#define RENDERING_VULKANRENDERER_ENABLE_VALIDATION_LAYERS

namespace Arclight::Rendering {

class VulkanRenderer final : public Renderer {
    friend class VulkanPipeline;
    friend class VulkanTexture;

public:
    ~VulkanRenderer();

    int initialize(WindowContext* windowContext);

    void render();
    void wait_device_idle() const;

    void resize_viewport(const Vector2i& pixelSize) override;

    // Get the default RenderPipeline
    RenderPipeline& default_pipeline();

    void bind_pipeline(RenderPipeline::PipelineHandle pipeline) override;
    void bind_texture(Texture::TextureHandle texture) override;
    void bind_vertex_buffer(void* buffer) override;

    // Draw Primitives
    void draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform = Matrix4());

    const std::string& get_name() const { return m_rendererName; }

    RenderPipeline::PipelineHandle
    create_pipeline(const Shader& vertexShader, const Shader& fragmentShader,
                   const RenderPipeline::PipelineFixedConfig& config) override;
    void destroy_pipeline(RenderPipeline::PipelineHandle handle) override;

    Texture::TextureHandle allocate_texture(const Vector2u& bounds, Texture::Format format) override;
    void update_texture(Texture::TextureHandle texture, const void* data) override;
    void destroy_texture(Texture::TextureHandle texture) override;

    void* allocate_vertex_buffer(unsigned vertexCount) override;
    void update_vertex_buffer(void* buffer, const Vertex* data) override;
    void* get_vertex_buffer_mapping(void* buffer) override;
    void destroy_vertex_buffer(void* buffer) override;

    constexpr VkFormat TextureToVkFormat(Texture::Format format) {
        switch (format) {
        case Texture::Format_RGBA8_SRGB:
            return VK_FORMAT_R8G8B8A8_SRGB;
        case Texture::Format_RGB8_SRGB:
            return VK_FORMAT_R8G8B8_SRGB;
        case Texture::Format_A8_SRGB:
            return VK_FORMAT_R8_SRGB;
        default:
            assert(!"Invalid texture format");
            return VK_FORMAT_UNDEFINED;
        }
    }

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

    struct VertexBuffer {
        VmaAllocation allocation; // VmaAlloaction of vertex buffer
        VkBuffer buffer;          // VkBuffer object

        void* hostMapping; // Host memory mapping of the buffer
        uint32_t size;     // How many vertexes can fit in buffer
    };

    std::mutex m_bufferDestroyLock;
    std::vector<std::pair<VkBuffer, VmaAllocation>> m_buffersPendingDestruction;

    inline VkDevice GetDevice() { return m_device; }
    inline VkRenderPass GetRenderPass() { return m_renderPass; }
    inline VkExtent2D GetScreenExtent() const { return m_swapExtent; }
    inline VmaAllocator Allocator() { return m_alloc; }

    inline OneTimeCommandBuffer CreateOneTimeCommandBuffer() {
        return OneTimeCommandBuffer(*this, m_commandPools[m_currentFrame]);
    }

    void do_draw_call(unsigned firstVertex, unsigned vertexCount, const Matrix4& transform) override;

    std::set<VulkanTexture*> m_textures;
    std::set<VulkanPipeline*> m_pipelines;
    std::set<VertexBuffer*> m_vertexBuffers;
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

    struct Frame {
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence fence;

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

    struct DepthBuffer {
        VkFormat depthFormat;
        VkImage depthImage;
        VmaAllocation depthImageAllocation;
        VkImageView depthImageView;
    } m_depthBuffer;

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

    void CreateDepthBuffer();
    void DestroyDepthBuffer();

    void _create_vertex_buffer(VertexBuffer* buffer, uint32_t vertexCount);
    void _destroy_vertex_buffer(VertexBuffer* buffer);

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

    uint32_t m_imageIndex = 0;   // Vulkan image index of frame being rendered
    unsigned m_currentFrame = 0; // Our index of current frame being rendered
    Frame m_frames[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];

    VulkanPipeline* m_lastPipelines[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VulkanPipeline* m_boundPipeline = nullptr;

    // Last used texture per pipeline
    // If the texture of the vertices being rendered is the same as the previous,
    // We do not need to update descriptor sets
    std::unordered_map<VulkanTexture*, VkDescriptorSet> m_textureDescriptorSets;
    VulkanTexture* m_lastTextures[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
    VulkanTexture* m_boundTexture = nullptr;

    VertexBuffer* m_boundVertexBuffer = nullptr;

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
