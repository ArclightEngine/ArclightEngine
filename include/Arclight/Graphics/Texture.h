#pragma once

#include <string>

#include <Arclight/Image.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Texture {
public:
    Texture();
    Texture(const Vector2u& bounds);
    Texture(const Image& image);
    Texture(const uint8_t* pixelData, const Vector2u& bounds);
    
private:
    Vector2u size = {0, 0};
};

} // namespace Arclight