#pragma once

#include "Graphics/Rendering/Pipeline.h"
#include "Graphics/Vertex.h"

#include <vulkan/vulkan_core.h>

namespace Arclight::Rendering {

class VulkanPipeline final {
public:
	struct PushConstant2DTransform{
		float transform[16]; // mat4 transform
	};

	VulkanPipeline(class VulkanRenderer& renderer, const Shader& vertexShader, const Shader& fragmentShader, const RenderPipeline::PipelineFixedConfig& config);
	~VulkanPipeline();

	VkPipeline GetPipelineHandle() { return m_pipeline; }

	void UpdatePushConstant(VkCommandBuffer commandBuffer, uint32_t offset, uint32_t size, const void* data);

	static VkPolygonMode ToVkPolygonMode(RenderPipeline::RasterizerConfig::PolygonMode mode);
	static VkPrimitiveTopology ToVkPrimitiveTopology(RenderPipeline::PrimitiveType type);
private:
	VkShaderModule CreateShaderModule(const Shader& shader);

	class VulkanRenderer& m_renderer;
	VkDevice m_device;

	VkRenderPass m_renderPass = VK_NULL_HANDLE;

	VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;

	VkVertexInputBindingDescription m_binding = {
		.binding = 0, // Index of the binding
		.stride = sizeof(Vertex), // Bytes from one entry to next
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX, // Next data entry after each vertex
	};

	static_assert(sizeof(Vertex::position.x) == sizeof(uint32_t));
	static_assert(sizeof(Vertex::position) == 8U);
	static_assert(sizeof(Vertex::colour) == 16U);
	VkVertexInputAttributeDescription m_attributeDescriptions[2] = {
		{
			.location = 0, // Shader input, specified using the location directive in shader code. We take position in location 0
			.binding = 0, // Index of the binding
			.format = VK_FORMAT_R32G32_SFLOAT, // Two 32-bit floats
			.offset = (offsetof(Vertex, position)),
		},
		{
			.location = 1, // Shader input, specified using the location directive in shader code. We take colour in location 1
			.binding = 0, // Index of the binding
			.format = VK_FORMAT_R32G32B32A32_SFLOAT, // 32-bit red, green, blue, alpha
			.offset = (offsetof(Vertex, colour)),
		}
	};

	static const VkPipelineVertexInputStateCreateInfo vertexInputStateDefault;
	static const VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateDefault;
	static const VkPipelineMultisampleStateCreateInfo multisampleStateDefault;
	static const VkPipelineRasterizationStateCreateInfo rasterizationStateDefault;
	static const VkPipelineColorBlendAttachmentState colourBlendAttachmentStateDefault;
	static const VkPipelineColorBlendStateCreateInfo colourBlendStateDefault;
};

} // namespace Arclight::Rendering