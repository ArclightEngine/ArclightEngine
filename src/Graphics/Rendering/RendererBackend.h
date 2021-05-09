#pragma once

#include <string>

#include "Pipeline.h"
#include "Graphics/Vertex.h"

namespace Arclight::Rendering {

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual int Initialize(class WindowContext* context) = 0;
	static inline Renderer* Instance() { return s_rendererInstance; }

	virtual void Render() = 0;
	virtual void WaitDeviceIdle() const = 0;

	virtual void Draw(const Vertex* vertices, unsigned vertexCount, const RenderPipeline& pipeline = RenderPipeline::Default()) = 0;

	virtual RenderPipeline::PipelineHandle CreatePipeline(const Shader& vertexShader, const Shader& fragmentShader, const RenderPipeline::PipelineFixedConfig& config) = 0;
	virtual void DestroyPipeline(RenderPipeline::PipelineHandle handle) = 0;

	virtual const RenderPipeline& DefaultPipeline() = 0;

	virtual const std::string& GetName() const = 0;

protected:
	static Renderer* s_rendererInstance;
};

} // namespace Arclight::Rendering