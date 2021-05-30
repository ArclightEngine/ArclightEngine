#include <Arclight/Graphics/Texture.h>

#include <Arclight/Graphics/Rendering/RendererBackend.h>

namespace Arclight {

Texture::Texture() {

}

Texture::Texture(const Vector2u& bounds)
    : m_size(bounds) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size);
}

Texture::Texture(const Image& image)
    : m_size({static_cast<unsigned int>(image.Size().x), static_cast<unsigned int>(image.Size().y)}) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size);

    Rendering::Renderer::Instance()->UpdateTexture(m_handle, image.Data());
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