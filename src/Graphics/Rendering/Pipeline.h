#pragma once

#include "ShaderBase.h"

namespace Arclight::Rendering {

class RenderPipeline {
public:
	struct RasterizerConfig {
		enum PolygonMode {
			PolygonInvalid,
			PolygonFill,
			PolygonLine,
		};

		PolygonMode polygonMode;
	};

	struct ColourBlending {
		enum BlendOp {
			BlendOpInvalid,
			BlendOpAdd,
		};

		BlendOp colourBlendOp;
		BlendOp alphaBlendOp;
		bool enabled;
	};

	struct PipelineFixedConfig {
		RasterizerConfig rasterizer;
		ColourBlending blending;
	};

	RenderPipeline() = default;
	virtual ~RenderPipeline() = default;

	static const PipelineFixedConfig defaultConfig;
private:
};

} // namespace Arclight::Rendering