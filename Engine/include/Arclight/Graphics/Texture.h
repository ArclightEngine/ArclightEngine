#pragma once

#include <string>

#include <Arclight/Graphics/Image.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Texture : NonCopyable {
public:
    enum Format {
        Format_RGBA8_SRGB = 0, // RGBA 8 bit
        Format_RGB8_SRGB,      // RGB 8 bit
        Format_A8_SRGB,        // Alpha only 8 bit
    };

    static constexpr unsigned formatSizes[] = {
        4, // RGBA 8 bit
        3, // RGB 8 bit
        1, // Alpha only 8 bit
    };

    using TextureHandle = void*;

    Texture();
    Texture(Texture&&);

    Texture(const Vector2u& bounds, Format format = Format_RGBA8_SRGB);
    Texture(const Image& image);
    Texture(const uint8_t* pixelData, const Vector2u& bounds, Format format = Format_RGBA8_SRGB);

    ~Texture();

    Texture& operator=(Texture&& other);

    void Update(const uint8_t* pixelData);
    void Load(const Image& image);
    void Reallocate(const Vector2u& bounds, Format format = Format_RGBA8_SRGB);

    inline const Vector2u& Size() const { return m_size; }
    inline Vector2f FloatSize() const {
        return {static_cast<float>(m_size.x), static_cast<float>(m_size.y)};
    }
    inline TextureHandle handle() { return m_handle; }

private:
    Vector2u m_size = {0, 0};
    Format m_format = Format_RGBA8_SRGB;

    TextureHandle m_handle = nullptr;
};

} // namespace Arclight