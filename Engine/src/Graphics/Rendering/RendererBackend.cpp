#include <Arclight/Graphics/Rendering/Renderer.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/Job.h>
#include <Arclight/Core/ThreadPool.h>

#include <cassert>
namespace Arclight::Rendering {

void Renderer::render() {
    // Each pipeline has a queue
    // For each entry bind texture -> bind vertex buffer -> perform call
    // In future may want to order by texture, etc.
    std::scoped_lock lockQueue(m_draw_queue_mutex);

    for (auto& bucket : m_queues) {
        bind_pipeline(bucket.first);

        auto& q = bucket.second;

        DrawCall* call = q.calls;
        while (q.callCount) {
            if (call->texture) {
                bind_texture(call->texture);
            }

            bind_vertex_buffer(call->vertexBuffer);
            do_draw_call(call->firstVertex, call->vertexCount, call->transform, call->view);

            call++;
            q.callCount--;
        }

        assert(!q.callCount);
    }
}

void Renderer::draw(void* vertexBuffer, unsigned firstVertex, unsigned vertexCount,
                    const Matrix4& transform, const Matrix4& view, Texture::TextureHandle texture,
                    RenderPipeline::PipelineHandle renderPipeline) {
    if(!renderPipeline) {
        renderPipeline = RenderPipeline::default_pipeline().handle();
    }
    
    std::scoped_lock lockQueue(m_draw_queue_mutex);

    auto& q = m_queues[renderPipeline];
    if(q.callCount >= q.size) {
        if(q.calls) {
            q.size += 1000;
            q.calls = (DrawCall*)realloc(q.calls, q.size * sizeof(DrawCall));
            if(q.calls == nullptr) {
                FatalRuntimeError("realloc failed!");
            }
        } else {
            q.size = 8000;
            q.calls = (DrawCall*)malloc(q.size * sizeof(DrawCall));
            assert(q.calls);
        }
    }

    assert(q.callCount < q.size);
    q.calls[q.callCount++] = DrawCall{firstVertex, vertexCount, transform, view, vertexBuffer, texture};
}

void Renderer::destroy_pipeline(RenderPipeline::PipelineHandle handle) {
    assert(handle);

    std::scoped_lock lockQueue(m_draw_queue_mutex);
    m_queues.erase(handle);
}

Renderer* Renderer::s_rendererInstance = nullptr;

} // namespace Arclight::Rendering
