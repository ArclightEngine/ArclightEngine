#pragma once

#include <Arclight/Colour.h>
#include <Arclight/Core/UnicodeString.h>
#include <Arclight/Core/Util.h>
#include <Arclight/Graphics/Font.h>
#include <Arclight/Graphics/Rect.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Vector.h>

#include <memory>

namespace Arclight {

class Text final : NonCopyable {
public:
    Text();
    //Text(Text&& other);
    Text(Text&& other) = default;
    Text(const UnicodeString& text);

    Text& operator=(Text&& other) = default;

    void SetFont(std::shared_ptr<Font> font);
    void SetColour(const Colour& colour);
    void SetText(UnicodeString text);
    void SetFontSize(int size);

    ALWAYS_INLINE Texture& Tex() { return m_texture; }
    ALWAYS_INLINE const Vertex* Vertices() const { return m_vertices; }
    ALWAYS_INLINE const Vector2f Bounds() const { return m_bounds.end; }

    Transform transform;

private:
    void Render();

    std::shared_ptr<Font> m_font = nullptr;

    // Freetype is used to render the text to this texture
    // The texture contains the final result of what is rendered on screen
    Texture m_texture;
    Vertex m_vertices[4] = {
        {{0, 1.0f}, {0.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}},    // Bottom left
        {{0, 0}, {0.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},       // Top left
        {{1.0f, 1.0f}, {1.0f, 1.0f}, {1.f, 1.f, 1.f, 1.f}}, // Bottom right
        {{1.0f, 0}, {1.0f, 0.0f}, {1.f, 1.f, 1.f, 1.f}},    // Top right
    };

    UnicodeString m_text; // Text to render
    Rectf m_bounds;       // Bounds of the text
    int m_pixelSize = 24;
};

} // namespace Arclight