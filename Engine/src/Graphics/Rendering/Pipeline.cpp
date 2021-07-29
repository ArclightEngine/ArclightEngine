#include <Arclight/Graphics/Rendering/Pipeline.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

namespace Arclight::Rendering {

RenderPipeline::RenderPipeline(const Shader& vertexShader, const Shader& fragmentShader, const PipelineFixedConfig& config){
    m_handle = Renderer::Instance()->CreatePipeline(vertexShader, fragmentShader, config);
}

RenderPipeline& RenderPipeline::Default() {
    return Renderer::Instance()->DefaultPipeline();
}

const RenderPipeline::PipelineFixedConfig RenderPipeline::defaultConfig = {
    .rasterizer = {
        .polygonMode = RasterizerConfig::PolygonFill,
    },
    .blending = {
        .colourBlendOp = ColourBlending::BlendOpAdd,
        .alphaBlendOp = ColourBlending::BlendOpAdd,
        .enabled = false,
    },
    .topology = PrimitiveTriangleStrip,
};

} // namespace Arclight::Rendering