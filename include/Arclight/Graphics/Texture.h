#pragma once

#include <string>

#include <Arclight/Graphics/Image.h>
#include <Arclight/Graphics/Rendering/RendererBackend.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Texture {
public:
    Texture();
    Texture(const Vector2u& bounds);
    Texture(const Image& image);
    Texture(const uint8_t* pixelData, const Vector2u& bounds);
    
    ~Texture();

    void Resize(const Vector2u& bounds);

private:
    Vector2u m_size = {0, 0};

    Rendering::Renderer::TextureHandle m_handle = nullptr;
};

} // namespace Arclight