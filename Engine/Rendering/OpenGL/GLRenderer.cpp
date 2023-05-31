#include <Rendering/OpenGL/GLRenderer.h>

#include <Arclight/Core/ThreadPool.h>
#include <Arclight/Core/Fatal.h>
#include <Arclight/Graphics/Transform.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/Window/WindowContext.h>

#include <cassert>

#include <SDL2/SDL_opengles2.h>

#ifdef ARCLIGHT_PLATFORM_WASM
#include <emscripten/html5.h>
#endif

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

int GLRenderer::initialize(class WindowContext* context) {
    m_windowContext = context;

    // OpenGL ES 3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
#ifdef ARCLIGHT_PLATFORM_WASM
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    m_glContext = SDL_GL_CreateContext(context->GetWindow());
    if (!m_glContext) {
        FatalRuntimeError("Failed to get OpenGL context from SDL: {}", SDL_GetError());
    }

    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.minorVersion = 0;

    auto glContext = emscripten_webgl_create_context("#canvas", &attr);
    assert(glContext != 0);
#else
    // Use OpenGL ES 3.1 on native platforms
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // When multithreading is enabled, create a second context that can be used
    // to update vertex buffers, textures, etc.
    if(Platform::multithreading_enabled()) {
        SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

        m_glContext = SDL_GL_CreateContext(context->GetWindow());
        m_glStreamContext = SDL_GL_CreateContext(context->GetWindow());

        SDL_GL_MakeCurrent(m_windowContext->GetWindow(), m_glContext);
    } else {
        m_glContext = SDL_GL_CreateContext(context->GetWindow());
    }

    if (!m_glContext) {
        FatalRuntimeError("Failed to get OpenGL context from SDL: {}", SDL_GetError());
    }

    if(SDL_GL_SetSwapInterval(1)) {
        FatalRuntimeError("Failed to enable VSync: {}", SDL_GetError());
    }

    const GLubyte* versionString = glGetString(GL_VERSION);
    if (!versionString) {
        FatalRuntimeError("Failed to get OpenGL version string!");
    }
#endif

    glGenBuffers(1, &m_transformUBO);

    glBindBuffer(GL_UNIFORM_BUFFER, m_transformUBO);

    // Should only be updated when window is resized,
    // will be used many times
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Matrix4::s_identityMatrix), Matrix4::s_identityMatrix,
                 GL_STATIC_DRAW);

    UpdateViewportTransform();

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

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    return 0;
}

void GLRenderer::render() {
    std::unique_lock lockGL(m_glMutex);
    die_if_not_gl_thread();

    clear();

    m_boundVBO = 0;
    Renderer::render();

    SDL_GL_SwapWindow(m_windowContext->GetWindow());

    m_debugFrameCounter++;
}

void GLRenderer::clear() {
    auto& clearColour = WindowContext::instance()->backgroundColour;
    glClearColor(clearColour.r, clearColour.g, clearColour.b, clearColour.a);

    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRenderer::resize_viewport(const Vector2i& newPixelSize) {
    die_if_not_gl_thread();

    glViewport(0, 0, newPixelSize.x, newPixelSize.y);
    UpdateViewportTransform();
}

RenderPipeline::PipelineHandle
GLRenderer::create_pipeline(const Shader& vertexShader, const Shader& fragmentShader,
                            const RenderPipeline::PipelineFixedConfig&) {
    assert(vertexShader.GetStage() == Shader::VertexShader &&
           fragmentShader.GetStage() == Shader::FragmentShader);

    std::unique_lock lockGL(m_glMutex);
    die_if_not_gl_thread();

    GLPipeline* pipeline = new GLPipeline(vertexShader, fragmentShader);

    m_pipelines.insert(pipeline);
    return pipeline;
}

void GLRenderer::destroy_pipeline(RenderPipeline::PipelineHandle pipelineHandle) {
    std::unique_lock lockGL(m_glMutex);
    die_if_not_gl_thread();

    size_t erased = m_pipelines.erase(reinterpret_cast<GLPipeline*>(pipelineHandle));
    assert(erased ==
           1); // Erase returns the amount of pipelines erased, ensure that this is exactly 1

    delete reinterpret_cast<GLPipeline*>(pipelineHandle);
}

RenderPipeline& GLRenderer::default_pipeline() {
    assert(m_defaultPipeline.get());
    return *m_defaultPipeline;
}

void GLRenderer::bind_pipeline(RenderPipeline::PipelineHandle pipeline) {
    m_boundPipeline = reinterpret_cast<GLPipeline*>(pipeline);
    if (m_boundPipeline->GetGLProgram() != m_lastProgram) {
        glCheck(glUseProgram(m_boundPipeline->GetGLProgram()));

        m_lastProgram = m_boundPipeline->GetGLProgram();
    }
}

void GLRenderer::bind_texture(Texture::TextureHandle texture) {
    if(texture == m_boundTexture) {
        return;
    }

    if (texture) {
        GLTexture* tex = reinterpret_cast<GLTexture*>(texture);
        glActiveTexture(GL_TEXTURE0);
        glCheck(glBindTexture(GL_TEXTURE_2D, tex->id));

        if (tex->arclightFormat == Texture::Format_A8_SRGB) {
            glUniform1i(m_boundPipeline->TextureFormatIndex(), 1);
        } else {
            glUniform1i(m_boundPipeline->TextureFormatIndex(), 0);
        }
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    m_boundTexture = (GLTexture*)texture;
}

void GLRenderer::bind_vertex_buffer(void* buffer) {
    GLVertexBuffer* vbo = (GLVertexBuffer*)buffer;

    m_boundVBO = vbo->id;
}

void* GLRenderer::allocate_vertex_buffer(unsigned vertexCount) {
    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    GLuint id;
    glCheck(glGenBuffers(1, &id));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, id));
    // GL_STREAM_DRAW - "Data modified once and used a few times"
    // GL_DYNAMIC_DRAW - "contents will be modified repeatedly and used many times"
    glCheck(glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), NULL, GL_DYNAMIC_DRAW));

    GLVertexBuffer* vbo = new GLVertexBuffer;
    vbo->id = id;
    vbo->vertexCount = vertexCount;

    m_vbos.insert(vbo);

    return (void*)vbo;
}

void GLRenderer::update_vertex_buffer(void* buffer, unsigned int offset, unsigned int size,
                                      const Vertex* vertices) {
    GLVertexBuffer* vbo = (GLVertexBuffer*)buffer;

    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    glCheck(glBindBuffer(GL_ARRAY_BUFFER, vbo->id));
    glCheck(glBufferSubData(GL_ARRAY_BUFFER, offset * sizeof(Vertex), size * sizeof(Vertex), vertices));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void* GLRenderer::get_vertex_buffer_mapping(void* buffer) { return nullptr; }

void GLRenderer::destroy_vertex_buffer(void* buffer) {
    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    GLVertexBuffer* vbo = (GLVertexBuffer*)buffer;

    size_t erased = m_vbos.erase(vbo);
    assert(erased ==
           1); // Erase returns the amount of pipelines erased, ensure that this is exactly 1

    glDeleteBuffers(1, &vbo->id);

    delete vbo;
}

void GLRenderer::do_draw_call(unsigned firstVertex, unsigned vertexCount, const Matrix4& transform,
                              const Matrix4& view) {
    auto vbo = GetVertexBufferObject(vertexCount);

    glBindVertexArray(m_boundPipeline->GetVAO());
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_boundVBO));

    // Position
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
    glEnableVertexAttribArray(0);
    // Texture Coordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(1);
    // Vertex Colour
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)offsetof(Vertex, colour));
    glEnableVertexAttribArray(2);

    glUniformMatrix4fv(m_boundPipeline->ModelTransformIndex(), 1, GL_FALSE, transform.matrix());
    glUniformMatrix4fv(m_boundPipeline->CanvasTransformIndex(), 1, GL_FALSE, view.matrix());

    glDrawArrays(GL_TRIANGLE_STRIP, firstVertex, vertexCount);
}

Texture::TextureHandle GLRenderer::allocate_texture(const Vector2u& size, Texture::Format format) {
    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    GLuint texID;
    glCheck(glGenTextures(1, &texID));

    glCheck(glBindTexture(GL_TEXTURE_2D, texID));

    GLenum glFormat = TextureToGLFormat(format);

    // TODO: Allow configuration of texture filtering and mipmapping
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    glCheck(glTexStorage2D(GL_TEXTURE_2D, 1, glFormat, size.x, size.y));

    GLTexture* tex = new GLTexture{texID, size, glFormat, format};
    m_textures.insert(tex);

    // Unbind texture
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
    return tex;
}

void GLRenderer::update_texture(Texture::TextureHandle texHandle, const void* data) {
    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    GLTexture* tex = reinterpret_cast<GLTexture*>(texHandle);
    assert(m_textures.contains(tex));

    glCheck(glBindTexture(GL_TEXTURE_2D, tex->id));
    GLuint nonSizedFormat;
    switch (tex->format) {
    case GL_RGBA8:
        nonSizedFormat = GL_RGBA;
        break;
    case GL_RGB8:
        nonSizedFormat = GL_RGB;
        break;
    case GL_R8:
        nonSizedFormat = GL_RED;
        break;
    default:
        FatalRuntimeError("Invalid texture format");
    }

    glCheck(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, tex->size.x, tex->size.y, nonSizedFormat,
                            GL_UNSIGNED_BYTE, data));

    // Unbind texture
    glCheck(glBindTexture(GL_TEXTURE_2D, 0));
}

void GLRenderer::destroy_texture(Texture::TextureHandle texHandle) {
    std::unique_lock lockGL(m_glMutex);
    acquire_stream_context_if_necessary();

    GLTexture* tex = reinterpret_cast<GLTexture*>(texHandle);

    size_t erased = m_textures.erase(tex);
    assert(erased ==
           1); // Erase returns the amount of textures erased, ensure that this is exactly 1

    // Delete OpenGL texture
    glDeleteTextures(1, &tex->id);

    // Delete our texture object
    delete tex;
}

bool GLRenderer::is_gl_thread() {
    return SDL_GL_GetCurrentContext() == m_glContext;
}

void GLRenderer::die_if_not_gl_thread() {
    if(SDL_GL_GetCurrentContext() != m_glContext) {
        FatalRuntimeError("Current thread does not hold the OpenGL context ({}): {}", (void*)SDL_GL_GetCurrentContext(), SDL_GetError());
    }
}

void GLRenderer::acquire_stream_context_if_necessary() {
    if(!is_gl_thread()) {
        if(SDL_GL_MakeCurrent(m_windowContext->GetWindow(), m_glStreamContext)) {
            FatalRuntimeError("Failed to acquire secondary GL context!");
        }
    }
}

void GLRenderer::UpdateViewportTransform() {
    m_viewportTransform = Transform2D(
        {-1, 1}, {2.f / m_windowContext->get_size().x, -2.f / m_windowContext->get_size().y});

    // Bind a range in the buffer to an index
    // We have a uniform buffer,
    // Binding to index 1,
    // Using buffer m_transform
    // Offset of 0
    // Size of one matrix
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, m_transformUBO, 0, sizeof(Matrix4::s_identityMatrix));

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4::s_identityMatrix),
                    m_viewportTransform.matrix().matrix());

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

GLRenderer::GLVBO GLRenderer::GetVertexBufferObject(unsigned vertexCount) {
    // For now, we only use one VBO, this is subject to change
    assert(vertexCount <= 4);
    return {m_vbo};
}

} // namespace Arclight::Rendering
