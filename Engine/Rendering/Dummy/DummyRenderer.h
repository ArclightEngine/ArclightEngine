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

    void resize_viewport(const Vector2i& newPixelSize) override {}
    void bind_texture(Texture::TextureHandle texture = nullptr) override {}
    void bind_vertex_buffer(void* buffer) override {}
    void bind_pipeline(RenderPipeline::PipelineHandle pipeline) override {}

    RenderPipeline::PipelineHandle
    create_pipeline(const Shader&, const Shader&,
                    const RenderPipeline::PipelineFixedConfig&) override {
        return nullptr;
    }

    void destroy_pipeline(RenderPipeline::PipelineHandle) override {}
    RenderPipeline& default_pipeline() override { return *m_defaultPipeline; }

    void draw(void* vertexBuffer, unsigned firstVertex, unsigned vertexCount,
              const Matrix4& transform, Texture::TextureHandle texture,
              RenderPipeline::PipelineHandle renderPipeline) {
        (void*)vertexBuffer;
        (void)firstVertex;
        (void)vertexCount;
        (void)transform;
        (void)texture;
        (void)renderPipeline;
    }

    Texture::TextureHandle allocate_texture(const Vector2u&, Texture::Format texFormat) {
        return nullptr;
    }
    void update_texture(Texture::TextureHandle, const void*) override {}
    void destroy_texture(Texture::TextureHandle) override{};

    void* allocate_vertex_buffer(unsigned vertexCount) override {}
    void update_vertex_buffer(void* buffer, const Vertex* vertices) override {}
    void* get_vertex_buffer_mapping(void* buffer) override { return nullptr; }
    void destroy_vertex_buffer(void* buffer) override {}

    const std::string& get_name() const override { return m_name; }

private:
    void do_draw_call(unsigned firstVertex, unsigned vertexCount, const Matrix4& transform) override {}

    const std::string m_name = "Dummy";

    Shader m_fragShader = Shader(Shader::FragmentShader);
    Shader m_vertShader = Shader(Shader::VertexShader);
    std::unique_ptr<RenderPipeline> m_defaultPipeline;
};

} // namespace Arclight::Rendering
