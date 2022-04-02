#pragma once

#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Util.h>
#include <Arclight/Graphics/Vertex.h>

namespace Arclight {

class VertexBuffer final : NonCopyable {
public:
    VertexBuffer() = default;
    VertexBuffer(unsigned size);
    VertexBuffer(VertexBuffer&&);

    VertexBuffer& operator=(VertexBuffer&& other);

    ~VertexBuffer();

    void update(const Vertex* vertices, unsigned int offset, unsigned int size);
    void reallocate(unsigned size);

    ALWAYS_INLINE unsigned size() const { return m_size; }
    ALWAYS_INLINE void* handle() { return m_handle; }

private:
    unsigned m_size = 0;

    // Pointer to object in the renderer
    void* m_handle = nullptr;

    Vertex* get_mapping();
};

} // namespace Arclight
