#include <Arclight/Graphics/Texture.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

#include <cassert>

namespace Arclight {

Texture::Texture() {}

Texture::Texture(Texture&& other)
    : m_size(other.m_size), m_format(other.m_format), m_handle(other.m_handle) {
        other.m_handle = nullptr;
    }

Texture::Texture(const Vector2u& bounds, Format format) : m_size(bounds), m_format(format) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size, format);
}

Texture::Texture(const Image& image) { Load(image); }

Texture::Texture(const uint8_t* pixelData, const Vector2u& bounds, Format format)
    : m_size(bounds), m_format(format) {
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size, format);
}

Texture::~Texture() {
    if (m_handle) {
        Rendering::Renderer::Instance()->DestroyTexture(m_handle);
    }
}

Texture& Texture::operator=(Texture&& other) {
    m_size = other.m_size;
    m_format = other.m_format;
    m_handle = other.m_handle;
    other.m_handle = nullptr;
    return *this;
}

void Texture::Update(const uint8_t* pixelData) {
    assert(m_handle);

    Rendering::Renderer::Instance()->UpdateTexture(m_handle, pixelData);
}

void Texture::Load(const Image& image) {
    if (m_handle) {
        Rendering::Renderer::Instance()->DestroyTexture(m_handle);
        m_handle = nullptr;
    }

    m_format = Format_RGBA8_SRGB;

    m_size = {static_cast<unsigned int>(image.Size().x), static_cast<unsigned int>(image.Size().y)};
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size, Format_RGBA8_SRGB);

    Rendering::Renderer::Instance()->UpdateTexture(m_handle, image.Data());
}

void Texture::Reallocate(const Vector2u& bounds, Format format) {
    m_size = bounds;

    if (m_handle) {
        Rendering::Renderer::Instance()->DestroyTexture(m_handle);
        m_handle = nullptr;
    }
    m_handle = Rendering::Renderer::Instance()->AllocateTexture(m_size, format);
}

} // namespace Arclight