#pragma once

#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Graphics/Transform.h>
#include <Arclight/Platform/Platform.h>

// OpenGL ES / WebGL Renderer
// Desktop OpenGL not supported at this time,
// due to Vulkan support

#include <GLES3/gl3.h>

#include <SDL2/SDL_opengles2.h>
#include <SDL2/SDL_video.h>

#include <cassert>
#include <mutex>

namespace Arclight::Rendering {

class GLRenderer final : public Renderer {
public:
    GLRenderer() = default;
    ~GLRenderer() override;

    int Initialize(class WindowContext* context) override;

    void WaitDeviceIdle() const override { glFinish(); }

    void Render() override;

    void Clear() override;

    void ResizeViewport(const Vector2i& pixelSize) override;

    RenderPipeline::PipelineHandle
    CreatePipeline(const Shader&, const Shader&,
                   const RenderPipeline::PipelineFixedConfig&) override;

    void DestroyPipeline(RenderPipeline::PipelineHandle) override;
    RenderPipeline& DefaultPipeline() override;

    void BindPipeline(RenderPipeline::PipelineHandle pipeline) override;
    void BindTexture(Texture::TextureHandle texture) override;

    void Draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform = Matrix4());
    Texture::TextureHandle AllocateTexture(const Vector2u& size, Texture::Format format) override;
    void UpdateTexture(Texture::TextureHandle, const void*) override;
    void DestroyTexture(Texture::TextureHandle) override;

    const std::string& GetName() const override { return m_name; }
    
    constexpr GLenum TextureToGLFormat(Texture::Format format) {
        switch (format) {
        case Texture::Format_RGBA8_SRGB:
            return GL_RGBA8;
        case Texture::Format_RGB8_SRGB:
            return GL_RGB8;
        case Texture::Format_A8_SRGB:
            return GL_R8;
        default:
            assert(!"Invalid texture format");
            return GL_RGBA8;
        }
    }

private:
    struct GLTexture {
        GLuint id;

        Vector2u size;
        GLenum format;
    };

    struct GLVBO {
        GLuint id;
    };

    // Update the viewport transform,
    // called on init and resize
    void UpdateViewportTransform();

    // Helps prevent rebinding the program across draw calls
    GLuint m_lastProgram;
    class GLPipeline* m_boundPipeline;

    // Uniform Buffer Object for the viewport transform
    GLuint m_transformUBO;

    GLVBO GetVertexBufferObject(unsigned vertexCount);
    GLuint m_vbo;

    WindowContext* m_windowContext;
    Transform m_viewportTransform;

    SDL_GLContext m_glContext;

    std::mutex m_glMutex;

#ifdef ARCLIGHT_PLATFORM_WASM
    const std::string m_name = "WebGL 2 (emulating OpenGL ES 3.0)";
#else
    const std::string m_name = "OpenGL ES 3.0";
#endif

    std::unique_ptr<RenderPipeline> m_defaultPipeline;
    std::set<class GLPipeline*> m_pipelines;

    std::set<GLTexture*> m_textures;
};

} // namespace Arclight::Rendering
