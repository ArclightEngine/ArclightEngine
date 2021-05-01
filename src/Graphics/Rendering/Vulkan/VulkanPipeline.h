#pragma once

#include "Graphics/Rendering/Pipeline.h"
#include "VulkanShader.h"

#include <vulkan/vulkan_core.h>

namespace Arclight::Rendering {

class VulkanPipeline final
	: public RenderPipeline {
public:
	VulkanPipeline(class VulkanRenderer& renderer, VulkanShader* vertexShader, VulkanShader* fragmentShader, const PipelineFixedConfig& rasterizerConfig = RenderPipeline::defaultConfig);
	~VulkanPipeline();

	VkPipeline GetPipelineHandle() { return m_pipeline; }

	static VkPolygonMode ToVkPolygonMode(RasterizerConfig::PolygonMode mode);
private:
	class VulkanRenderer& m_renderer;
	VkDevice m_device;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;

	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;

	static const VkPipelineVertexInputStateCreateInfo vertexInputStateDefault;
	static const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateDefault;
	static const VkPipelineMultisampleStateCreateInfo multisampleStateDefault;
	static const VkPipelineRasterizationStateCreateInfo rasterizationStateDefault;
	static const VkPipelineColorBlendAttachmentState colourBlendAttachmentStateDefault;
	static const VkPipelineColorBlendStateCreateInfo colourBlendStateDefault;
};

} // namespace Arclight::Rendering