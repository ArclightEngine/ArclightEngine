#include <Arclight/Graphics/Rendering/RendererBackend.h>

namespace Arclight::Rendering {

void Renderer::Draw(RenderObject& obj){
	obj.Draw(*this);
}

Renderer* Renderer::s_rendererInstance = nullptr;

} // namespace Rendering