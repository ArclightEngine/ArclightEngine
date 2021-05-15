#pragma once

#include <Arclight/Vector.h>
#include <Arclight/Transform.h>
#include <Arclight/Texture.h>

namespace Arclight {

class Sprite
    : public Transform {
public:
    Sprite();
    Sprite(Texture& texture);

private:
    Texture* m_texture = nullptr;
};

} // namespace Arclight