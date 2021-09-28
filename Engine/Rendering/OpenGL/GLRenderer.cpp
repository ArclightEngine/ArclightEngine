#include <Rendering/OpenGL/GLRenderer.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/Window/WindowContext.h>

#include <cassert>

#include <SDL2/SDL_opengles2.h>

#include "GLCheck.h"
#include "GLPipeline.h"

#include "DefaultShaderSource.h"

namespace Arclight::Rendering {

GLRenderer::~GLRenderer() {
    for (auto* p : m_pipelines) {
        delete p;
    }

    for (auto* t : m_textures) {
        delete t;
    }

    m_pipelines.clear();
    m_textures.clear();

    SDL_GL_DeleteContext(m_glContext);
}

int GLRenderer::Initialize(class WindowContext* context) {
    // OpenGL ES 3.1
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    m_windowContext = context;

    m_glContext = SDL_GL_CreateContext(context->GetWindow());
    if (!m_glContext) {
        FatalRuntimeError("Failed to get OpenGL context from SDL!");
    }

    const GLubyte* versionString = glGetString(GL_VERSION);
    if (!versionString) {
        FatalRuntimeError("Failed to get OpenGL version string!");
    }

    s_rendererInstance = this;

    {
        // Default vertex and fragment shaders
        Shader vertShader(Shader::VertexShader, defaultVertexShaderData);
        Shader fragShader(Shader::FragmentShader, defaultFragmentShaderData);

        m_defaultPipeline = std::make_unique<RenderPipeline>(vertShader, fragShader);
    }

    auto& clearColour = context->backgroundColour;
    glClearColor(clearColour.r, clearColour.g, clearColour.b, clearColour.a);
    return 0;
}

void GLRenderer::Render() {
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_GL_SwapWindow(m_windowContext->GetWindow());
}

RenderPipeline::PipelineHandle
GLRenderer::CreatePipeline(const Shader& vertexShader, const Shader& fragmentShader,
                           const RenderPipeline::PipelineFixedConfig&) {
    assert(vertexShader.GetStage() == Shader::VertexShader &&
           fragmentShader.GetStage() == Shader::FragmentShader);

    std::unique_lock lockGL(m_glMutex);
    Logger::Debug("creating pipeline!");

    GLPipeline* pipeline = new GLPipeline(vertexShader, fragmentShader);

    m_pipelines.insert(pipeline);
    return pipeline;
}

void GLRenderer::DestroyPipeline(RenderPipeline::PipelineHandle pipelineHandle) {
    std::unique_lock lockGL(m_glMutex);

    size_t erased = m_pipelines.erase(reinterpret_cast<GLPipeline*>(pipelineHandle));
    assert(erased ==
           1); // Erase returns the amount of pipelines erased, ensure that this is exactly 1

    delete reinterpret_cast<GLPipeline*>(pipelineHandle);
}

RenderPipeline& GLRenderer::DefaultPipeline() {
    assert(m_defaultPipeline.get());
    return *m_defaultPipeline;
}

Texture::TextureHandle GLRenderer::AllocateTexture(const Vector2u& size) {
    std::unique_lock lockGL(m_glMutex);

    GLuint texID;
    glCheck(glGenTextures(1, &texID));

    glCheck(glBindTexture(GL_TEXTURE_2D, texID));

    // TODO: Allow configuration of texture filtering and mipmapping
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, size.x, size.y));

    GLTexture* tex = new GLTexture{texID, size};
    m_textures.insert(tex);

    // Unbind texture
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    return tex;
}

void GLRenderer::UpdateTexture(Texture::TextureHandle texHandle, const void* data) {
    std::unique_lock lockGL(m_glMutex);

    GLTexture* tex = reinterpret_cast<GLTexture*>(texHandle);
    assert(m_textures.contains(tex));

    glCheck(glBindTexture(GL_TEXTURE_2D, tex->id));

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->size.x, tex->size.y, GL_RGBA, GL_UNSIGNED_BYTE,
                    data);

    // Unbind texture
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLRenderer::DestroyTexture(Texture::TextureHandle texHandle) {
    std::unique_lock lockGL(m_glMutex);

    GLTexture* tex = reinterpret_cast<GLTexture*>(texHandle);

    size_t erased = m_textures.erase(tex);
    assert(erased ==
           1); // Erase returns the amount of textures erased, ensure that this is exactly 1

    // Delete OpenGL texture
    glDeleteTextures(1, &tex->id);

    // Delete our texture object
    delete tex;
}

} // namespace Arclight::Rendering
