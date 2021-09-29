#include <Rendering/OpenGL/GLRenderer.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Graphics/Transform.h>
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
    // OpenGL ES 3.0
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_windowContext = context;

    m_glContext = SDL_GL_CreateContext(context->GetWindow());
    if (!m_glContext) {
        FatalRuntimeError("Failed to get OpenGL context from SDL: ", SDL_GetError());
    }

    const GLubyte* versionString = glGetString(GL_VERSION);
    if (!versionString) {
        FatalRuntimeError("Failed to get OpenGL version string!");
    }

    m_viewportTransform = Transform(
        {-1, 1}, {2.f / m_windowContext->GetSize().x, -2.f / m_windowContext->GetSize().y});

    glGenBuffers(1, &m_transformUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);
    // Should only be updated when window is resized,
    // will be used many times
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix4::s_identityMatrix), Matrix4::s_identityMatrix,
                 GL_STATIC_DRAW);

    // Bind a range in the buffer to an index
    // We have a uniform buffer,
    // Binding to index 1,
    // Using buffer m_transform
    // Offset of 0
    // Size of one matrix
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_transformUBO, 0, sizeof(Matrix4::s_identityMatrix));

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4::s_identityMatrix),
                    m_viewportTransform.Matrix().Matrix());

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glGenBuffers(1, &m_vbo);
    assert(m_vbo);

    // Enable alpha blending
    glEnable(GL_BLEND);  
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
    SDL_GL_SwapWindow(m_windowContext->GetWindow());

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT);
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

void GLRenderer::Draw(const Vertex* vertices, unsigned vertexCount, const Matrix4& transform,
                      Texture::TextureHandle texture, RenderPipeline& pipeline) {
    GLPipeline* glPipeline = reinterpret_cast<GLPipeline*>(pipeline.Handle());

    if (texture) {
        GLTexture* tex = reinterpret_cast<GLTexture*>(texture);
        glActiveTexture(GL_TEXTURE0);
        glCheck(glBindTexture(GL_TEXTURE_2D, tex->id));
    }

    if(glPipeline->GetGLProgram() != m_lastProgram){
        glCheck(glUseProgram(glPipeline->GetGLProgram()));

        m_lastProgram = glPipeline->GetGLProgram();
    }

    auto vbo = GetVertexBufferObject(vertexCount);
    // GL_STREAM_DRAW - "Data modified once and used a few times"
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, vbo.id));

    glBindVertexArray(glPipeline->GetVAO());

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    glEnableVertexAttribArray(0);
    // Texture Coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    // Vertex Colour
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(2);

    glCheck(glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), vertices, GL_STREAM_DRAW));

    // It is assumed the uniform object for the transform is at location 0
    glUniformMatrix4fv(0, 1, GL_FALSE, transform.Matrix());

    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

    if (texture) { 
        glBindTexture(GL_TEXTURE_2D, 0);
    }
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

GLRenderer::GLVBO GLRenderer::GetVertexBufferObject(unsigned vertexCount) {
    // For now, we only use one VBO, this is subject to change
    assert(vertexCount <= 4);
    return {m_vbo};
}

} // namespace Arclight::Rendering
