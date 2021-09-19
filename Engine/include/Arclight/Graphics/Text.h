#pragma once

#include <Arclight/Colour.h>
#include <Arclight/Core/UnicodeString.h>
#include <Arclight/Graphics/Font.h>
#include <Arclight/Graphics/Rendering/RenderObject.h>
#include <Arclight/Graphics/Texture.h>
#include <Arclight/Graphics/Transform.h>
#include <Arclight/Graphics/Vertex.h>
#include <Arclight/Vector.h>

#include <memory>

namespace Arclight {

class Text : public Rendering::RenderObject {
public:
    enum {
        FontSize_Pixels,
        FontSize_Pt,
    };

    Text();
    Text(const UnicodeString& text);

    void SetColour(const Colour& colour);
    void SetText(const UnicodeString& text);
    void SetFontSize(int size, int type = FontSize_Pixels);

    const Texture& Tex() const { return m_texture; }

    Transform transform;

private:
    void Draw() override;

    Font* font;

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
    int m_fontSize = 12;
    int m_fontSizeType = FontSize_Pixels;
};

} // namespace Arclight