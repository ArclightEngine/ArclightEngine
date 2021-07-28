#pragma once

#include <Arclight/Graphics/Rendering/RenderObject.h>

#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Graphics/Rect.h>

#include <Arclight/Vector.h>

#include <cassert>

namespace Arclight {

class Sprite
    : public Rendering::RenderObject {
public:
    Sprite();
    Sprite(Texture& texture);

    void Draw();

    inline void SetScale(float scaleX, float scaleY) {
        m_transform.SetScale(scaleX, scaleY);
        if(m_texture) {
            m_scaledTextureSize = { m_texture->FloatSize().x * m_transform.GetScale().x,
            m_texture->FloatSize().y * m_transform.GetScale().y };
        }
    }

    inline void SetScale(const Vector2f& scale) { SetScale(scale.x, scale.y); }

    inline void SetPosition(float x, float y) { m_transform.SetPosition(x, y); }
    inline void SetPosition(const Vector2f& pos) { m_transform.SetPosition(pos); }
    inline void Move(float offsetX, float offsetY) { m_transform.SetPosition(m_transform.GetPosition().x + offsetX, m_transform.GetPosition().y + offsetY); }
    inline void Move(const Vector2f& offset) { m_transform.SetPosition(m_transform.GetPosition() + offset); }

    inline void SetColour(const Vector4f& colour) {
        m_vertices[0].colour = colour;
        m_vertices[1].colour = colour;
        m_vertices[2].colour = colour;
        m_vertices[3].colour = colour;
    }

    inline const Vector2f& Scale() const { return m_transform.GetScale(); }
    inline const Vector2f& Position() const { return m_transform.GetPosition(); }	// Draw Drawable

    inline Rectf Bounds() const { return { m_transform.GetPosition(), m_transform.GetPosition() + m_scaledTextureSize }; }

private:
    Texture* m_texture = nullptr;
    Vector2f m_scaledTextureSize = { 0, 0 };

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