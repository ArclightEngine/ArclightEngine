#pragma once

#include <Arclight/Graphics/Rendering/Shader.h>
#include <Arclight/Core/NonCopyable.h>

namespace Arclight::Rendering {

class RenderPipeline
	: NonCopyable {
public:
	using PipelineHandle = void*;

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

	enum PrimitiveType {
		PrimitiveTriangleList, // List of separate triangles
		PrimitiveTriangleStrip, // List of connected triangles, each triangle shares the last two vertices
	};

	struct PipelineFixedConfig {
		RasterizerConfig rasterizer;
		ColourBlending blending;
		PrimitiveType topology;
	};

	RenderPipeline(const Shader& vertexShader, const Shader& fragmentShader, const PipelineFixedConfig& config = defaultConfig);
	virtual ~RenderPipeline() = default;

	inline PipelineHandle Handle() { return m_handle; }

	static RenderPipeline& Default();

	static const PipelineFixedConfig defaultConfig;
private:
	PipelineHandle m_handle;
};

} // namespace Arclight::Rendering