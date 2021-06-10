#include <Arclight/Graphics/Rendering/RenderObject.h>

#include <Arclight/Graphics/Rendering/RendererBackend.h>

namespace Arclight::Rendering {

RenderObject::RenderObject()
    : m_renderer(*Renderer::Instance()) {
    m_renderer.RegisterRenderObject(this);
}

RenderObject::~RenderObject(){
    m_renderer.DeregisterRenderObject(this);
}

} // namespace Arclight