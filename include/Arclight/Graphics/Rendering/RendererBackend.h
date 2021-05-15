#pragma once

#include <string>

#include <Arclight/Graphics/Rendering/Pipeline.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Graphics/Transform.h>

namespace Arclight::Rendering {

class Renderer {
public:
	using TextureHandle = void*;

	virtual ~Renderer() = default;

	virtual int Initialize(class WindowContext* context) = 0;
	static inline Renderer* Instance() { return s_rendererInstance; }

	virtual void Render() = 0;
	virtual void WaitDeviceIdle() const = 0;

	virtual void Draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform = Matrix4(), RenderPipeline& pipeline = RenderPipeline::Default()) = 0;

	////////////////////////////////////////
	/// \brief CreatePipeline
	///
	///	Create a new render pipeline.
	///
	///	\param vertexShader Shader object for vertex shader code
	/// \param fragmentShader Shader object for fragment shader code
	/// \param config Render pipeline configuration
	///
	/// \return Handle to pipeline, pipeline handles are specific to the renderer and are no more than a way to unqiuely identify pipelines internally
	////////////////////////////////////////
	virtual RenderPipeline::PipelineHandle CreatePipeline(const Shader& vertexShader, const Shader& fragmentShader, const RenderPipeline::PipelineFixedConfig& config) = 0;

	////////////////////////////////////////
	/// \brief DestroyPipeline
	///
	///	Destroy render pipeline. MUST be a valid handle.
	///
	/// \param handle Valid pipeline handle
	////////////////////////////////////////
	virtual void DestroyPipeline(RenderPipeline::PipelineHandle handle) = 0;
	virtual RenderPipeline& DefaultPipeline() = 0;

	////////////////////////////////////////
	/// \brief AllocateTexture
	///
	/// \param bounds Texture bounds. Enough space to store pixels in RGBA format is allocated
	///
	/// \return Handle to texture, texture handles are specific to the renderer and are no more than a way to unqiuely identify textures internally
	////////////////////////////////////////
	virtual TextureHandle AllocateTexture(const Vector2u& bounds) = 0;

	////////////////////////////////////////
	/// \brief UpdateTexture
	///
	/// \param texture Texture handle. MUST be valid
	/// \param data Pointer to pixel data in RGBA format. MUST be large enough to contain full texture data.
	////////////////////////////////////////
	virtual void UpdateTexture(TextureHandle texture, const void* data) = 0;
	
	////////////////////////////////////////
	/// \brief DestroyTexture
	///
	///	Destroy texture. MUST be a valid handle.
	///
	/// \param texture Valid texture handle
	////////////////////////////////////////
	virtual void DestroyTexture(TextureHandle texture) = 0;

	virtual const std::string& GetName() const = 0;

protected:
	static Renderer* s_rendererInstance;
};

} // namespace Arclight::Rendering