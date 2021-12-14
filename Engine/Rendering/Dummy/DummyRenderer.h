#pragma once

#include <Arclight/Graphics/Rendering/Pipeline.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Rendering/Shader.h>

#include <memory>

namespace Arclight::Rendering {

class DummyRenderer final : public Renderer {
public:
    DummyRenderer() = default;

    int initialize(class WindowContext* context) override {
        s_rendererInstance = this;

        m_defaultPipeline = std::make_unique<RenderPipeline>(m_vertShader, m_fragShader);
        return 0;
    }

    void wait_device_idle() const override {}

    RenderPipeline::PipelineHandle
    create_pipeline(const Shader&, const Shader&,
                   const RenderPipeline::PipelineFixedConfig&) override {
        return nullptr;
    }

    void destroy_pipeline(RenderPipeline::PipelineHandle) override {}
    RenderPipeline& default_pipeline() override { return *m_defaultPipeline; }

    void draw(const Vertex*, unsigned, const Matrix4& transform = Matrix4(),
              Texture::TextureHandle texture = nullptr,
              RenderPipeline& pipeline = RenderPipeline::Default()) {
        (void)transform;
        (void)texture;
        (void)pipeline;
    }

    Texture::TextureHandle allocate_texture(const Vector2u&) { return nullptr; }
    void update_texture(Texture::TextureHandle, const void*) override {}
    void destroy_texture(Texture::TextureHandle) override{};

    const std::string& get_name() const override { return m_name; }

private:
    const std::string m_name = "Dummy";

    Shader m_fragShader = Shader(Shader::FragmentShader);
    Shader m_vertShader = Shader(Shader::VertexShader);
    std::unique_ptr<RenderPipeline> m_defaultPipeline;
};

} // namespace Arclight::Rendering
