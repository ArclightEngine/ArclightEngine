#pragma once

#include <Arclight/Core/Util.h>
#include <Arclight/Graphics/Rect.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Vector.h>

namespace Arclight {

struct Sprite {
    class Texture* texture = nullptr;
    Vertex vertices[4] = {
        {{0, 1.0f}, {0.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}},    // Bottom left
        {{0, 0}, {0.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},       // Top left
        {{1.0f, 1.0f}, {1.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}}, // Bottom right
        {{1.0f, 0}, {1.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},    // Top right
    };

    ALWAYS_INLINE Vector2f pixel_size_unscaled() const {
        return vertices[3].position;
    }
};

ALWAYS_INLINE Sprite create_sprite(const Vector2i& size, const Rectf& textureCoordinates = Rectf(Vector2f{1.f}),
                                  const Vector4f& colour = {1.f, 1.f, 1.f, 1.f}) {
    Sprite sprite{
        .texture = nullptr,
        .vertices = {{{0.f, static_cast<float>(size.y)},
                      {textureCoordinates.left, textureCoordinates.bottom},
                      colour},
                     {{0.f, 0.f}, {textureCoordinates.left, textureCoordinates.top}, colour},
                     {{static_cast<float>(size.x), static_cast<float>(size.y)},
                      {textureCoordinates.right, textureCoordinates.bottom},
                      colour},
                     {{static_cast<float>(size.x), 0.f},
                      {textureCoordinates.right, textureCoordinates.top},
                      colour}}};

    return sprite;
}

} // namespace Arclight