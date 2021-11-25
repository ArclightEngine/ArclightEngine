#include "VulkanPipeline.h"

#include "VulkanRenderer.h"
#include <Arclight/Graphics/Vertex.h>

#include <cassert>
#include <stdexcept>

namespace Arclight::Rendering {

VulkanPipeline::VulkanPipeline(VulkanRenderer& renderer, const Shader& vertexShader,
                               const Shader& fragmentShader,
                               const RenderPipeline::PipelineFixedConfig& config)
    : m_renderer(renderer), m_device(renderer.GetDevice()), m_renderPass(renderer.GetRenderPass()) {
    assert(m_device != VK_NULL_HANDLE);
    assert(m_renderPass != VK_NULL_HANDLE);

    assert(vertexShader.GetStage() == Shader::VertexShader);
    assert(fragmentShader.GetStage() == Shader::FragmentShader);

    VkPushConstantRange pushConstant2D = {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(PushConstant2DTransform),
    };

    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &m_renderer.m_descriptorSetLayout,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstant2D,
    };

    if (vkCreatePipelineLayout(m_device, &vkPipelineLayoutCreateInfo, nullptr, &m_pipelineLayout)) {
        throw std::runtime_error(
            "[Fatal error] VulkanPipeline::VulkanPipeline: failed to create pipeline layout!");
    }

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &m_binding,
        .vertexAttributeDescriptionCount = 3,
        .pVertexAttributeDescriptions = m_attributeDescriptions,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = inputAssemblyStateDefault;
    inputAssembly.topology = ToVkPrimitiveTopology(config.topology);

    VkPipelineMultisampleStateCreateInfo multisampling = multisampleStateDefault;

    VkPipelineRasterizationStateCreateInfo rasterizer = rasterizationStateDefault;
    rasterizer.polygonMode = ToVkPolygonMode(config.rasterizer.polygonMode);

    VkPipelineColorBlendAttachmentState colourBlendAttachment = colourBlendAttachmentStateDefault;
    VkPipelineColorBlendStateCreateInfo colourBlending = colourBlendStateDefault;
    colourBlending.attachmentCount = 1;
    colourBlending.pAttachments = &colourBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil = depthStencilStateDefault;

    VkShaderModule vertexModule = CreateShaderModule(vertexShader);
    VkShaderModule fragmentModule = CreateShaderModule(fragmentShader);

    const int shaderStageCount = 2;
    VkPipelineShaderStageCreateInfo shaderStages[shaderStageCount] = {
        {
            // Vertex Stage
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        },
        {
            // Fragment Stage
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentModule,
            .pName = "main",
            .pSpecializationInfo = nullptr,
        }};

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
        .pViewports = nullptr, // Otherwise, &viewport, however we set this dyanimically
        .scissorCount = 1,
        .pScissors = nullptr, // Otherwise, &scissor, however we set this dynamically
    };

    // VK_DYNAMIC_STATE_VIEWPORT specifies that the pViewports state in
    // VkPipelineViewportStateCreateInfo will be ignored Must be set with vkCmdSetViewport before
    // any drawing commands Same goes with VK_DYNAMIC_STATE_SCISSOR This allows us to resize the
    // viewport without having to recreate the pipeline
    VkDynamicState dynamicStates[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = sizeof(dynamicStates) / sizeof(VkDynamicState),
        .pDynamicStates = dynamicStates,
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
        .pDepthStencilState = &depthStencil,
        .pColorBlendState = &colourBlending,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_pipelineLayout,
        .renderPass = m_renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = 0,
    };

    if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gfxPipelineCreateInfo, nullptr,
                                  &m_pipeline) != VK_SUCCESS) {
        throw std::runtime_error(
            "VulkanRenderer::initialize: Failed to create Vulkan graphics pipeline!");
    }

    auto cmdBuf = m_renderer.CreateOneTimeCommandBuffer();

    // Bind our pipeline and set the viewport and scissor
    vkCmdBindPipeline(cmdBuf.Buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdSetViewport(cmdBuf.Buffer(), 0, 1, &viewport);
    vkCmdSetScissor(cmdBuf.Buffer(), 0, 1, &scissor);
    // Our buffer will be submitted on destruction, so we don't need to do anything here

    vkDestroyShaderModule(m_device, vertexModule, nullptr);
    vkDestroyShaderModule(m_device, fragmentModule, nullptr);
}

VulkanPipeline::~VulkanPipeline() {
    VkDevice device = m_renderer.GetDevice();

    vkDestroyPipeline(device, m_pipeline, nullptr);
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
}

void VulkanPipeline::UpdatePushConstant(VkCommandBuffer commandBuffer, uint32_t offset,
                                        uint32_t size, const void* data) {
    vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, offset, size,
                       data);
}

VkShaderModule VulkanPipeline::CreateShaderModule(const Shader& shader) {
    VkShaderModuleCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = shader.DataSize(),
        .pCode = reinterpret_cast<const uint32_t*>(shader.GetData()),
    };

    VkShaderModule sModule;
    if (vkCreateShaderModule(m_device, &createInfo, nullptr, &sModule)) {
        throw std::runtime_error(
            "[Fatal error] VulkanShader::CreateModule: Failed to create shader module!");
    }

    return sModule;
}

VkPolygonMode VulkanPipeline::ToVkPolygonMode(RenderPipeline::RasterizerConfig::PolygonMode mode) {
    switch (mode) {
    case RenderPipeline::RasterizerConfig::PolygonFill:
        return VK_POLYGON_MODE_FILL;
    case RenderPipeline::RasterizerConfig::PolygonLine:
        return VK_POLYGON_MODE_LINE;
    default:
        throw std::runtime_error(
            "[Fatal error] VulkanPipeline::ToVkPolygonMode: Invalid polygon mode!");
        return VK_POLYGON_MODE_FILL;
    }
}

VkPrimitiveTopology VulkanPipeline::ToVkPrimitiveTopology(RenderPipeline::PrimitiveType type) {
    switch (type) {
    case RenderPipeline::PrimitiveTriangleList:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    case RenderPipeline::PrimitiveTriangleStrip:
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    default:
        throw std::runtime_error(
            "[Fatal error] VulkanPipeline::ToVkPrimitiveTopology: Invalid primitive type!");
        return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    }
}

const VkPipelineInputAssemblyStateCreateInfo VulkanPipeline::inputAssemblyStateDefault = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
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
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA, // Use colour blending using alpha
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // No colour blending
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
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
    }};

const VkPipelineDepthStencilStateCreateInfo VulkanPipeline::depthStencilStateDefault = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext = nullptr,
    .flags = 0,
    .depthTestEnable = VK_TRUE,
    .depthWriteEnable = VK_TRUE,
    .depthCompareOp = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable = VK_FALSE,
    .front = {},
    .back = {},
    // This option allows fragments falling outside these values to be discarded,
    // not in use
    .minDepthBounds = 0.f,
    .maxDepthBounds = 1.f,
};

} // namespace Arclight::Rendering