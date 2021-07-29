#pragma once

#include <string>

#include <Arclight/Graphics/Image.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Texture {
public:
	using TextureHandle = void*;

    Texture();
    Texture(const Vector2u& bounds);
    Texture(const Image& image);
    Texture(const uint8_t* pixelData, const Vector2u& bounds);
    
    ~Texture();

    void Load(const Image& image);
    void Resize(const Vector2u& bounds);

    inline const Vector2u& Size() const { return m_size; }
    inline Vector2f FloatSize() const { return { static_cast<float>(m_size.x), static_cast<float>(m_size.y) }; }
    inline TextureHandle Handle() { return m_handle; }

private:
    Vector2u m_size = {0, 0};

    TextureHandle m_handle = nullptr;
};

} // namespace Arclight