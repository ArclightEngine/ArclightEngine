#include "Pipeline.h"

namespace Arclight::Rendering {

const RenderPipeline::PipelineFixedConfig RenderPipeline::defaultConfig = {
    .rasterizer = {
        .polygonMode = RasterizerConfig::PolygonFill,
    },
    .blending = {
        .colourBlendOp = ColourBlending::BlendOpAdd,
        .alphaBlendOp = ColourBlending::BlendOpAdd,
        .enabled = false,
    }
};

} // namespace Arclight::Rendering