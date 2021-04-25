#pragma once

#include <vulkan/vulkan.h>

#include "Window/WindowContext.h"
#include "../RendererBackend.h"

#include <vector>
#include <optional>

namespace Rendering {

class VulkanRenderer final : public RendererBackend {
public:
	~VulkanRenderer();

	int Initialize(WindowContext* windowContext);

	const std::string& GetName() const { return m_rendererName; }

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

	const std::string m_rendererName = "Vulkan";

	WindowContext* m_windowContext = nullptr;
	VkSurfaceKHR m_surface; // Vulkan surface
	VkInstance m_instance; // Vulkan instance
	VkSwapchainKHR m_swapchain; // Vulkan swap chain

	VkFormat m_swapImageFormat = VK_FORMAT_UNDEFINED;
	VkColorSpaceKHR m_swapColourSpace;
	VkExtent2D m_swapExtent;
	std::vector<VkImage> m_images; // Swapchain image handles

	std::vector<VkImageView> m_imageViews;

	VkDevice m_device = VK_NULL_HANDLE; // Logical device
	VkQueue m_graphicsQueue; // Graphics queue
	uint32_t m_graphicsQueueFamily;

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
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = 0,
		.ppEnabledExtensionNames = nullptr,
	};
};

} // namespace Rendering
