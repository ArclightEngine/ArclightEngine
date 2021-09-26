#pragma once

#include <Arclight/Graphics/Rendering/Pipeline.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Rendering/Shader.h>

#include <memory>

namespace Arclight::Rendering {

class DummyRenderer final : public Renderer {
public:
    DummyRenderer() = default;

    int Initialize(class WindowContext* context) override {
        s_rendererInstance = this;

        m_defaultPipeline = std::make_unique<RenderPipeline>(m_vertShader, m_fragShader);
        return 0;
    }

    void WaitDeviceIdle() const override {}

    RenderPipeline::PipelineHandle
    CreatePipeline(const Shader&, const Shader&,
                   const RenderPipeline::PipelineFixedConfig&) override {
        return nullptr;
    }

    void DestroyPipeline(RenderPipeline::PipelineHandle) override {}
    RenderPipeline& DefaultPipeline() override { return *m_defaultPipeline; }

    void Draw(const Vertex*, unsigned, const Matrix4& transform = Matrix4(),
              Texture::TextureHandle texture = nullptr,
              RenderPipeline& pipeline = RenderPipeline::Default()) {
        (void)transform;
        (void)texture;
        (void)pipeline;
    }

    Texture::TextureHandle AllocateTexture(const Vector2u&) { return nullptr; }
    void UpdateTexture(Texture::TextureHandle, const void*) override {}
    void DestroyTexture(Texture::TextureHandle) override{};

    const std::string& GetName() const override { return m_name; }

private:
    const std::string m_name = "Dummy";

    Shader m_fragShader = Shader(Shader::FragmentShader);
    Shader m_vertShader = Shader(Shader::VertexShader);
    std::unique_ptr<RenderPipeline> m_defaultPipeline;
};

} // namespace Arclight::Rendering
