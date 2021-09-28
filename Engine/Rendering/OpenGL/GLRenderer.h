#pragma once

#include <Arclight/Graphics/Rendering/Renderer.h>

// OpenGL ES / WebGL Renderer
// Desktop OpenGL not supported at this time,
// due to Vulkan support

#include <GLES3/gl3.h>

namespace Arclight::Rendering {

class GLRenderer final : public Renderer {
public:
    GLRenderer() = default;

    int Initialize(class WindowContext* context) override;

    void WaitDeviceIdle() const override {}

    RenderPipeline::PipelineHandle
    CreatePipeline(const Shader&, const Shader&,
                   const RenderPipeline::PipelineFixedConfig&) override;

    void DestroyPipeline(RenderPipeline::PipelineHandle) override;
    RenderPipeline& DefaultPipeline() override;

    void Draw(const Vertex*, unsigned, const Matrix4& transform = Matrix4(),
              Texture::TextureHandle texture = nullptr,
              RenderPipeline& pipeline = RenderPipeline::Default()) {
        (void)transform;
        (void)texture;
        (void)pipeline;
    }

    Texture::TextureHandle AllocateTexture(const Vector2u&);
    void UpdateTexture(Texture::TextureHandle, const void*) override;
    void DestroyTexture(Texture::TextureHandle) override;

    const std::string& GetName() const override { return m_name; }

private:
    const std::string m_name = "OpenGL ES 3";

    

    Shader m_fragShader = Shader(Shader::FragmentShader);
    Shader m_vertShader = Shader(Shader::VertexShader);
    std::unique_ptr<RenderPipeline> m_defaultPipeline;
};

} // namespace Arclight::Rendering
