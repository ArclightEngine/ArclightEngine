#include <Arclight/Graphics/Sprite.h>

#include <Arclight/Graphics/Rendering/RendererBackend.h>

namespace Arclight {

Sprite::Sprite(){

}

Sprite::Sprite(Texture& texture) {
	m_texture = &texture;

	m_verticesDirty = true;
}

void Sprite::Draw(Rendering::Renderer& renderer){
	if(m_verticesDirty){ // Rebuild vertices
		m_vertices[0].position = { 0, static_cast<float>(m_texture->Size().y) };
		m_vertices[1].position = { 0, 0 };
		m_vertices[2].position = { static_cast<float>(m_texture->Size().x), static_cast<float>(m_texture->Size().y) };
		m_vertices[3].position = { static_cast<float>(m_texture->Size().x), 0 };

		m_verticesDirty = false;
	}

	renderer.Draw(m_vertices, 4, m_transform.Matrix(), m_texture->Handle());
}

} // namespace Arclight