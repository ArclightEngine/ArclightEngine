#pragma once

#include <Arclight/Graphics/Rendering/Renderer.h>

// OpenGL ES / WebGL Renderer
// Desktop OpenGL not supported at this time,
// due to Vulkan support

#include <GLES3/gl3.h>

#include <SDL2/SDL_video.h>
#include <SDL2/SDL_opengles2.h>

#include <mutex>

namespace Arclight::Rendering {

class GLRenderer final : public Renderer {
public:
    GLRenderer() = default;
    ~GLRenderer() override;

    int Initialize(class WindowContext* context) override;

    void WaitDeviceIdle() const override { glFinish(); }

    void Render() override;

    RenderPipeline::PipelineHandle
    CreatePipeline(const Shader&, const Shader&,
                   const RenderPipeline::PipelineFixedConfig&) override;

    void DestroyPipeline(RenderPipeline::PipelineHandle) override;
    RenderPipeline& DefaultPipeline() override;

    void Draw(const Vertex*, unsigned, const Matrix4& transform = Matrix4(),
              Texture::TextureHandle texture = nullptr,
              RenderPipeline& pipeline = RenderPipeline::Default()) override {
        (void)transform;
        (void)texture;
        (void)pipeline;
    }

    Texture::TextureHandle AllocateTexture(const Vector2u&) override;
    void UpdateTexture(Texture::TextureHandle, const void*) override;
    void DestroyTexture(Texture::TextureHandle) override;

    const std::string& GetName() const override { return m_name; }

private:
    struct GLTexture {
        GLuint id;
        Vector2u size;
    };

    WindowContext* m_windowContext;

    SDL_GLContext m_glContext;

    std::mutex m_glMutex;

    const std::string m_name = "OpenGL ES 3";

    std::unique_ptr<RenderPipeline> m_defaultPipeline;
    std::set<class GLPipeline*> m_pipelines;

    std::set<GLTexture*> m_textures;
};

} // namespace Arclight::Rendering
