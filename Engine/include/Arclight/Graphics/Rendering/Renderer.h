#pragma once

#include <mutex>
#include <set>
#include <stack>
#include <string>
#include <unordered_map>

#include <Arclight/Graphics/Rendering/Pipeline.h>

#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Vertex.h>

#include <Arclight/Platform/API.h>

namespace Arclight {
class WindowContext;

namespace Rendering {

class ARCLIGHT_API Renderer {
public:
    virtual ~Renderer() = default;

    virtual int initialize(class WindowContext* context) = 0;
    static inline Renderer* instance() { return s_rendererInstance; }

    virtual void render();
    virtual void wait_device_idle() const = 0;

    ////////////////////////////////////////
    /// \brief Clear the window using the set clear colour
    ///
    /// Not necessary on all platforms, with the notable exception of WebGL
    ////////////////////////////////////////
    virtual void clear() {}

    ////////////////////////////////////////
    /// \brief Resize the render viewport
    ///
    /// Called on window resize
    ////////////////////////////////////////
    virtual void resize_viewport(const Vector2i& newPixelSize) = 0;

    ////////////////////////////////////////
    /// \brief BInd texture to renderer
    ///
    /// Bind texture to be used on next draw call.
    ////////////////////////////////////////
    virtual void bind_texture(Texture::TextureHandle texture = nullptr) = 0;

    ////////////////////////////////////////
    /// \brief BInd vertex buffer to renderer
    ///
    /// Bind vertex buffer to be used on next draw call.
    ////////////////////////////////////////
    virtual void bind_vertex_buffer(void* buffer) = 0;

    ////////////////////////////////////////
    /// \brief BInd pipeline to renderer
    ///
    /// Bind pipeline to be used on next draw call.
    ////////////////////////////////////////
    virtual void bind_pipeline(
        RenderPipeline::PipelineHandle pipeline = instance()->default_pipeline().handle()) = 0;

    ////////////////////////////////////////
    /// \brief Draw
    ///
    ///	Draw a polygon. Generally corresponds to a draw call.
    ///
    /// \param vertexBuffer Handle of vertex buffer
    /// \param firstVertex First vertex in vertex buffer to draw
    /// \param vertexCount Amount of vertices to draw
    /// \param transform Transformation matrix to apply
    /// \param texture Texture to use in shader
    /// \param renderPipeline Render pipeline to use
    ////////////////////////////////////////
    virtual void draw(void* vertexBuffer, unsigned firstVertex, unsigned vertexCount,
                      const Matrix4& transform, const Matrix4& view, Texture::TextureHandle texture,
                      RenderPipeline::PipelineHandle renderPipeline);

    ////////////////////////////////////////
    /// \brief create_pipeline
    ///
    ///	Create a new render pipeline.
    ///
    ///	\param vertexShader Shader object for vertex shader code
    /// \param fragmentShader Shader object for fragment shader code
    /// \param config Render pipeline configuration
    ///
    /// \return Handle to pipeline, pipeline handles are specific to the renderer and are no more
    /// than a way to unqiuely identify pipelines internally
    ////////////////////////////////////////
    virtual RenderPipeline::PipelineHandle
    create_pipeline(const Shader& vertexShader, const Shader& fragmentShader,
                    const RenderPipeline::PipelineFixedConfig& config) = 0;

    ////////////////////////////////////////
    /// \brief DestroyPipeline
    ///
    ///	Destroy render pipeline. MUST be a valid handle.
    ///
    /// \param handle Valid pipeline handle
    ////////////////////////////////////////
    virtual void destroy_pipeline(RenderPipeline::PipelineHandle handle);
    virtual RenderPipeline& default_pipeline() = 0;

    ////////////////////////////////////////
    /// \brief allocate_texture
    ///
    /// \param bounds Texture bounds. Enough space to store pixels in RGBA format is allocated
    ///
    /// \return Handle to texture, texture handles are specific to the renderer and are no more than
    /// a way to unqiuely identify textures internally
    ////////////////////////////////////////
    virtual Texture::TextureHandle allocate_texture(const Vector2u& bounds,
                                                    Texture::Format texFormat) = 0;

    ////////////////////////////////////////
    /// \brief update_texture
    ///
    /// \param texture Texture handle. MUST be valid
    /// \param data Pointer to pixel data in relevant format. MUST be large enough to contain full
    /// texture data.
    ////////////////////////////////////////
    virtual void update_texture(Texture::TextureHandle texture, const void* data) = 0;

    ////////////////////////////////////////
    /// \brief destroy_texture
    ///
    ///	Destroy texture. MUST be a valid handle.
    ///
    /// \param texture Valid texture handle
    ////////////////////////////////////////
    virtual void destroy_texture(Texture::TextureHandle texture) = 0;

    ////////////////////////////////////////
    /// \brief allocate_vertex_buffer
    ///
    /// \param vertexCount Buffer size in vertices.
    ///
    /// \return Handle to vertex buffer
    ////////////////////////////////////////
    virtual void* allocate_vertex_buffer(unsigned vertexCount) = 0;

    ////////////////////////////////////////
    /// \brief update_vertex_buffer
    ///
    /// \param buffer Buffer handle. MUST be valid
    /// \param data Pointer to vertex data in relevant format. MUST be large enough to contain full
    /// vertex buffer data.
    ////////////////////////////////////////
    virtual void update_vertex_buffer(void* buffer, unsigned int offset, unsigned int size, const Vertex* vertices) = 0;

    virtual void* get_vertex_buffer_mapping(void* buffer) = 0;

    ////////////////////////////////////////
    /// \brief destroy_vertex_buffer
    ///
    ///	Destroy vertex buffer. MUST be a valid handle.
    ///
    /// \param buffer Valid vertex buffer handle
    ////////////////////////////////////////
    virtual void destroy_vertex_buffer(void* buffer) = 0;

    virtual const std::string& get_name() const = 0;

protected:
    struct DrawCall {
        unsigned firstVertex;
        unsigned vertexCount;
        Matrix4 transform;
        Matrix4 view;
        void* vertexBuffer;
        Texture::TextureHandle texture;
    };

    virtual void do_draw_call(unsigned firstVertex, unsigned vertexCount, const Matrix4& transform, const Matrix4& view) = 0;

    std::mutex m_draw_queue_mutex;

    // Do NOT preserve queue order, use stack
    std::unordered_map<void*, std::stack<DrawCall>> m_queues;

    static Renderer* s_rendererInstance;
};

} // namespace Rendering
} // namespace Arclight
