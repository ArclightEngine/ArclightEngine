#pragma once

#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Vector.h>

namespace Arclight {

struct Sprite {
    Transform transform = Transform();
    Vector2f size = {0, 0};
    class Texture* texture = nullptr;
    Vertex vertices[4] = {
        {{0, 1.0f}, {0.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}},    // Bottom left
        {{0, 0}, {0.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},       // Top left
        {{1.0f, 1.0f}, {1.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}}, // Bottom right
        {{1.0f, 0}, {1.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},    // Top right
    };
};

} // namespace Arclight