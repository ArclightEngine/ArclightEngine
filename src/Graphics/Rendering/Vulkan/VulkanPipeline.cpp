#include "VulkanPipeline.h"

#include "VulkanRenderer.h"

#include <stdexcept>
#include <cassert>

namespace Arclight::Rendering {

VulkanPipeline::VulkanPipeline(VulkanRenderer& renderer, VulkanShader* vertexShader, VulkanShader* fragmentShader, const PipelineFixedConfig& config)
	: m_renderer(renderer), m_device(renderer.GetDevice()), m_renderPass(renderer.GetRenderPass()) {
	assert(m_device != VK_NULL_HANDLE);
	assert(m_renderPass != VK_NULL_HANDLE);

	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.setLayoutCount = 0,
		.pSetLayouts = nullptr,
		.pushConstantRangeCount = 0,
		.pPushConstantRanges = nullptr,
	};

	if(vkCreatePipelineLayout(m_device, &vkPipelineLayoutCreateInfo, nullptr, &m_pipelineLayout)){
		throw std::runtime_error("[Fatal error] VulkanPipeline::VulkanPipeline: failed to create pipeline layout!");
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = vertexInputStateDefault;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = inputAssemblyStateDefault;
	VkPipelineMultisampleStateCreateInfo multisampling = multisampleStateDefault;

	VkPipelineRasterizationStateCreateInfo rasterizer = rasterizationStateDefault;
	rasterizer.polygonMode = ToVkPolygonMode(config.rasterizer.polygonMode);

	VkPipelineColorBlendAttachmentState colourBlendAttachment = colourBlendAttachmentStateDefault;
	VkPipelineColorBlendStateCreateInfo colourBlending = colourBlendStateDefault;
	colourBlending.attachmentCount = 1;
	colourBlending.pAttachments = &colourBlendAttachment;

	VkShaderModule vertexModule = vertexShader->CreateModule(m_device);
	VkShaderModule fragmentModule = fragmentShader->CreateModule(m_device);

	const int shaderStageCount = 2;
	VkPipelineShaderStageCreateInfo shaderStages[shaderStageCount] = {
		{ // Vertex Stage
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = vertexModule,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		},
		{ // Fragment Stage
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.pNext = nullptr,
			.flags = 0,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = fragmentModule,
			.pName = "main",
			.pSpecializationInfo = nullptr,
		}
	};

	VkExtent2D extent = m_renderer.GetScreenExtent();
	VkViewport viewport = {
		.x = 0.f,
		.y = 0.f,
		.width = static_cast<float>(extent.width),
		.height = static_cast<float>(extent.height),
		.minDepth = 0.f,
		.maxDepth = 1.f,
	};

	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = extent,
	};

	VkPipelineViewportStateCreateInfo viewportState = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor,
	};

	VkGraphicsPipelineCreateInfo gfxPipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.stageCount = shaderStageCount,
		.pStages = shaderStages,
		.pVertexInputState = &vertexInputInfo,
		.pInputAssemblyState = &inputAssembly,
		.pTessellationState = nullptr,
		.pViewportState = &viewportState,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = nullptr,
		.pColorBlendState = &colourBlending,
		.pDynamicState = nullptr,
		.layout = m_pipelineLayout,
		.renderPass = m_renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0,
	};

	if(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gfxPipelineCreateInfo, nullptr, &m_pipeline) != VK_SUCCESS){
		throw std::runtime_error("VulkanRenderer::Initialize: Failed to create Vulkan graphics pipeline!");
	}

	vkDestroyShaderModule(m_device, vertexModule, nullptr);
	vkDestroyShaderModule(m_device, fragmentModule, nullptr);
}

VulkanPipeline::~VulkanPipeline(){
	VkDevice device = m_renderer.GetDevice();

	vkDestroyPipeline(device, m_pipeline, nullptr);
	vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

VkPolygonMode VulkanPipeline::ToVkPolygonMode(RasterizerConfig::PolygonMode mode){
	switch(mode){
		case RasterizerConfig::PolygonFill:
			return VK_POLYGON_MODE_FILL;
		case RasterizerConfig::PolygonLine:
			return VK_POLYGON_MODE_LINE;
		default:
			throw std::runtime_error("[Fatal error] VulkanPipeline::ToVkPolygonMode: Invalid polygon mode!");
			return VK_POLYGON_MODE_FILL;
	}
}

const VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::inputAssemblyStateDefault = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	.primitiveRestartEnable = VK_FALSE,
};

const VkPipelineVertexInputStateCreateInfo VulkanPipeline::vertexInputStateDefault = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.vertexBindingDescriptionCount = 0,
	.pVertexBindingDescriptions = nullptr,
	.vertexAttributeDescriptionCount = 0,
	.pVertexAttributeDescriptions = nullptr,
};

const VkPipelineMultisampleStateCreateInfo VulkanPipeline::multisampleStateDefault = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
	.sampleShadingEnable = VK_FALSE,
	.minSampleShading = 1.f,
	.pSampleMask = nullptr,
	.alphaToCoverageEnable = VK_FALSE,
	.alphaToOneEnable = VK_FALSE,
};

const VkPipelineRasterizationStateCreateInfo VulkanPipeline::rasterizationStateDefault = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.depthClampEnable = VK_FALSE,
	.rasterizerDiscardEnable = VK_FALSE,
	.polygonMode = VK_POLYGON_MODE_FILL, // Render filled polygons
	.cullMode = VK_CULL_MODE_BACK_BIT,
	.frontFace = VK_FRONT_FACE_CLOCKWISE,
	.depthBiasEnable = VK_FALSE,
	.depthBiasConstantFactor = 0.f,
	.depthBiasClamp = 0.f,
	.depthBiasSlopeFactor = 0.f,
	.lineWidth = 1.f,
};

const VkPipelineColorBlendAttachmentState VulkanPipeline::colourBlendAttachmentStateDefault = {
	.blendEnable = VK_FALSE,
	.srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // No colour blending
	.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
	.colorBlendOp = VK_BLEND_OP_ADD,
	.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // No colour blending
	.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
	.alphaBlendOp = VK_BLEND_OP_ADD,
	.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};

const VkPipelineColorBlendStateCreateInfo VulkanPipeline::colourBlendStateDefault = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
	.pNext = nullptr,
	.flags = 0,
	.logicOpEnable = VK_FALSE,
	.logicOp = VK_LOGIC_OP_COPY,
	.attachmentCount = 0,
	.pAttachments = nullptr,
	.blendConstants = {
		0.f,
		0.f,
		0.f,
		0.f,
	}
};

} // namespace Arclight::Rendering