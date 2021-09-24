#include <Arclight/Graphics/Rendering/RenderObject.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

#include <cassert>

namespace Arclight::Rendering {

RenderObject::RenderObject() : m_renderer(*Renderer::Instance()) {
    assert(&m_renderer == Renderer::Instance());
    m_renderer.RegisterRenderObject(this);
}

RenderObject::RenderObject(const RenderObject& other) : m_renderer(other.m_renderer) {
    assert(&m_renderer == Renderer::Instance());
    m_renderer.RegisterRenderObject(this);
}

RenderObject::RenderObject(RenderObject&& other) : m_renderer(other.m_renderer) {
    assert(&m_renderer == Renderer::Instance());
    m_renderer.RegisterRenderObject(this);
}

RenderObject& RenderObject::operator=(const RenderObject& other) {
    assert(&m_renderer == Renderer::Instance());
    return *this;
}

RenderObject& RenderObject::operator=(RenderObject&& other) {
    assert(&m_renderer == Renderer::Instance());
    return *this;
}

RenderObject::~RenderObject() { m_renderer.DeregisterRenderObject(this); }

} // namespace Arclight::Rendering