#include <Arclight/Graphics/Rendering/RenderObject.h>

#include <Arclight/Graphics/Rendering/RendererBackend.h>

namespace Arclight::Rendering {

RenderObject::RenderObject()
    : m_renderer(*Renderer::Instance()) {
    m_renderer.RegisterRenderObject(this);
}

RenderObject::RenderObject(const RenderObject& other)
    : m_renderer(other.m_renderer) {
    m_renderer.RegisterRenderObject(this);
}

RenderObject::~RenderObject(){
    m_renderer.DeregisterRenderObject(this);
}

} // namespace Arclight