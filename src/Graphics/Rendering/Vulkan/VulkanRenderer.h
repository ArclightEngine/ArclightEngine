#pragma once

#include <vulkan/vulkan.h>

#include "Window/WindowContext.h"

#include "Graphics/Rendering/RendererBackend.h"
#include "VulkanShader.h"
#include "VulkanPipeline.h"

#include <vector>
#include <optional>
#include <map>

#define RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT 2

namespace Arclight::Rendering {

class VulkanRenderer final : public Renderer {
	friend class VulkanPipeline;
public:
	~VulkanRenderer();

	int Initialize(WindowContext* windowContext);

	void Draw();
	void WaitDeviceIdle() const;

	int LoadShader(const std::string& name);

	const std::string& GetName() const { return m_rendererName; }

protected:
	inline VkDevice GetDevice() { return m_device; }
	inline VkRenderPass GetRenderPass() { return m_renderPass; }
	inline VkExtent2D GetScreenExtent() const { return m_swapExtent; }

private:
	struct SwapChainInfo {
		VkSurfaceCapabilitiesKHR surfaceCapabilites;
		std::vector<VkSurfaceFormatKHR> surfaceFormats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	int LoadExtensions();

	int EnumerateGPUs();
	int SelectBestGPU();
	int GPUSuitability(VkPhysicalDevice dev);

	std::optional<uint32_t> GetGraphicsQueueFamily();
	SwapChainInfo GetSwapChainInfo();

	int CreateLogicalDevice();
	void CreateCommandPool();

	const std::string m_rendererName = "Vulkan";

	std::map<std::string, VulkanShader> m_shaders;

	WindowContext* m_windowContext = nullptr;
	VkSurfaceKHR m_surface; // Vulkan surface
	VkInstance m_instance; // Vulkan instance
	VkSwapchainKHR m_swapchain; // Vulkan swap chain

	VkRenderPass m_renderPass;
	VulkanPipeline* m_pipeline = nullptr;

	VkFormat m_swapImageFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_swapColourSpace;
	VkExtent2D m_swapExtent;

	unsigned currentFrame = 0; // Index of current frame being rendered
	VkSemaphore m_imageAvailableSemaphores[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
	VkSemaphore m_renderFinishedSemaphores[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];
	VkFence m_frameFences[RENDERING_VULKANRENDERER_MAX_FRAMES_IN_FLIGHT];

	std::vector<VkImage> m_images; // Swapchain image handles
	std::vector<VkImageView> m_imageViews;
	std::vector<VkFramebuffer> m_framebuffers; // Swapchain framebuffers

	VkDevice m_device = VK_NULL_HANDLE; // Logical device
	VkQueue m_graphicsQueue; // Graphics queue
	uint32_t m_graphicsQueueFamily;

	VkCommandPool m_commandPool; // Command pool
	std::vector<VkCommandBuffer> m_commandBuffers;

	std::vector<const char*> m_vkExtensions; // List of Vulkan extension names

	std::vector<VkPhysicalDevice> m_GPUs; // List of Vulkan Physical Devices (GPUs)
	VkPhysicalDevice m_renderGPU; // Our current GPU

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
		.enabledLayerCount = 1,
		.ppEnabledLayerNames = validationLayers,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
	};

	const char* const validationLayers[1] {
		"VK_LAYER_KHRONOS_validation"
	};
};

} // namespace Arclight::Rendering
