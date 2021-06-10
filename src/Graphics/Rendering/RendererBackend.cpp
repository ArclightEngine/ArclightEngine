#include <Arclight/Graphics/Rendering/RendererBackend.h>

#include <cassert>
#include <stdexcept>

namespace Arclight::Rendering {

void Renderer::Draw(RenderObject& obj){
	obj.Draw();
}

void Renderer::Render(){
	for(auto& obj : m_renderObjects){
		obj->Draw();
	}
}

void Renderer::RegisterRenderObject(RenderObject* obj){
	if(!m_renderObjects.insert(obj).second){ // Ensure it gets inserted
		throw std::runtime_error("Failed to insert RenderObject!");
	}
}

void Renderer::DeregisterRenderObject(RenderObject* obj){
	if(!m_renderObjects.erase(obj)){ // Ensure it gets removed/exists
		throw std::runtime_error("Failed to remove RenderObject!");
	}
}

Renderer* Renderer::s_rendererInstance = nullptr;

} // namespace Rendering