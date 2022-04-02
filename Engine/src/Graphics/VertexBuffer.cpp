#include <Arclight/Graphics/VertexBuffer.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

#include <assert.h>

namespace Arclight {

VertexBuffer::VertexBuffer(unsigned size) {
    m_handle = Rendering::Renderer::instance()->allocate_vertex_buffer(size);
    m_size = size;
}

VertexBuffer::VertexBuffer(VertexBuffer&& other) {
    assert(!m_handle);

    m_handle = other.m_handle;
    other.m_handle = nullptr;

    m_size = other.m_size;
    other.m_size = 0;
}

VertexBuffer& VertexBuffer::operator=(VertexBuffer&& other) {
    if (m_handle) {
        Rendering::Renderer::instance()->destroy_vertex_buffer(m_handle);
    }

    m_handle = other.m_handle;
    other.m_handle = nullptr;

    m_size = other.m_size;
    other.m_size = 0;

    return *this;
}

VertexBuffer::~VertexBuffer() {
    if (m_handle) {
        Rendering::Renderer::instance()->destroy_vertex_buffer(m_handle);
    }
}

Vertex* VertexBuffer::get_mapping() {
    assert(m_handle);

    auto* r = Rendering::Renderer::instance();

    return (Vertex*)r->get_vertex_buffer_mapping(m_handle);
}

void VertexBuffer::update(const Vertex* vertices, unsigned int offset, unsigned int size) {
    auto* r = Rendering::Renderer::instance();
    r->update_vertex_buffer(m_handle, offset, size, vertices);
}

void VertexBuffer::reallocate(unsigned size) {
    auto* r = Rendering::Renderer::instance();

    if(m_handle) {
        r->destroy_vertex_buffer(m_handle);
    }

    m_size = size;

    if(!size) {
        m_handle = nullptr;
        return;
    }

    m_handle = r->allocate_vertex_buffer(size);
}

} // namespace Arclight
