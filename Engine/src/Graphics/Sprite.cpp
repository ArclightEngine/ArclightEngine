#include <Arclight/Graphics/Sprite.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

namespace Arclight {

Sprite::Sprite(){

}

Sprite::Sprite(Texture& texture) {
	m_texture = &texture;

	m_scaledTextureSize = { m_texture->FloatSize().x * m_transform.GetScale().x,
	m_texture->FloatSize().y * m_transform.GetScale().y };

	m_verticesDirty = true;
}

void Sprite::Draw(){
	if(m_verticesDirty){ // Rebuild vertices
		m_vertices[0].position = { 0, static_cast<float>(m_texture->Size().y) };
		m_vertices[1].position = { 0, 0 };
		m_vertices[2].position = { static_cast<float>(m_texture->Size().x), static_cast<float>(m_texture->Size().y) };
		m_vertices[3].position = { static_cast<float>(m_texture->Size().x), 0 };

		m_verticesDirty = false;
	}
	
	m_renderer.Draw(m_vertices, 4, m_transform.Matrix(), m_texture->Handle());
}

} // namespace Arclight