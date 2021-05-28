#include <Arclight/Graphics/Texture.h>

namespace Arclight {

Texture::Texture() {

}

Texture::Texture(const Vector2u& bounds)
    : m_size(bounds) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size);
}

Texture::Texture(const Image& image) {

}

Texture::Texture(const uint8_t* pixelData, const Vector2u& bounds)
    : m_size(bounds) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size); 
}

Texture::~Texture() {
    if(m_handle){
        Rendering::Renderer::Instance()->DestroyTexture(m_handle);
    }
}

void Texture::Resize(const Vector2u& bounds){
    m_size = bounds;

    if(m_handle){
        Rendering::Renderer::Instance()->DestroyTexture(m_handle);
    }
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size);
}

} // namespace Arclight