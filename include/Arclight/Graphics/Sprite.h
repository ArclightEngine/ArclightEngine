#pragma once

#include <Arclight/Graphics/Rendering/RenderObject.h>

#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Vertex.h>

#include <Arclight/Vector.h>

namespace Arclight {

class Sprite
    : public Transform, public Rendering::RenderObject {
public:
    Sprite();
    Sprite(Texture& texture);

    void Draw(Rendering::Renderer& renderer);

    inline const Vector2f& Position() const { return m_transform.GetPosition(); }	// Draw Drawable

private:
    Texture* m_texture = nullptr;

    Transform m_transform;

    bool m_verticesDirty = true;
    Vertex m_vertices[4] = {
		{{0, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.f}}, // Bottom left
		{{0, 0}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.f}}, // Top left
		{{1.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.f}}, // Bottom right
		{{1.0f, 0}, {1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.f}}, // Top right
    };
};

} // namespace Arclight