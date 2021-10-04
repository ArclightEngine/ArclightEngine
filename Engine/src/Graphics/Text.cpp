#include <Arclight/Graphics/Text.h>

#include <cassert>
#include <stdexcept>

#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Texture.h>

#include <unicode/schriter.h>

#include "Freetype.h"

FreeType* FreeType::m_instance = new FreeType();

FreeType::FreeType() {
    assert(!m_instance);
    m_instance = this;

    Arclight::Logger::Debug("Initializing Freetype!");

    if (FT_Error e = FT_Init_FreeType(&m_library); e) {
        Arclight::Logger::Error("Error ", FT_Error_String(e), " initializing freetype!");
        throw std::runtime_error("Failed to initialize Freetype!");
    }
}

FT_Error FreeType::NewFace(Arclight::File* file, FT_Long index, FT_Face* outFace,
                           std::vector<uint8_t>& outData) {
    std::unique_lock lockFT(m_lock);

    size_t fSize = file->GetSize();
    outData.resize(fSize);

    if (file->Read(outData.data(), fSize) < 0) {
        return FT_Err_Cannot_Open_Resource;
    }

    return FT_New_Memory_Face(m_library, outData.data(), fSize, index, outFace);
}

FT_Error FreeType::DoneFace(FT_Face face) {
    std::unique_lock lockFT(m_lock);

    return FT_Done_Face(face);
}

namespace Arclight {

Text::Text() {}

/*Text::Text(Text&& other)
    : transform(std::move(other.transform)), m_font(std::move(other.m_font)),
      m_texture(std::move(other.m_texture)), m_text(std::move(other.m_text)),
      m_bounds(other.m_bounds), m_pixelSize(other.m_pixelSize) {}*/

Text::Text(const UnicodeString& text) { SetText(text); }

void Text::Render() {
    if (!m_font.get()) {
        return;
    }

    if (m_text.isEmpty()) {
        return;
    }

    std::unique_lock fontLock(m_font->m_lock);
    FT_Face face = reinterpret_cast<FT_Face>(m_font->m_handle);

    if (FT_Set_Pixel_Sizes(face, 0, m_pixelSize)) {
        Arclight::Logger::Error("Failed to set font size!");
        return;
    }

    // Compose a vector of glyphs
    std::vector<unsigned int> glyphs;
    icu::StringCharacterIterator it(m_text);
    UChar32 codepoint = it.next32PostInc();
    while (codepoint != icu::StringCharacterIterator::DONE) {
        glyphs.push_back(FT_Get_Char_Index(face, codepoint));
        codepoint = it.next32PostInc();
    }

    bool useKerning = FT_HAS_KERNING(face);
    // In 64ths of a pixel so r shift by 6
    int pixelLineHeight = face->size->metrics.height >> 6;
    Vector2u texBounds = {0, pixelLineHeight};

    unsigned int prevGlyph = 0;
    for (unsigned int glyph : glyphs) {
        if(glyph == '\r'){
            continue;
        } else if(glyph == '\n'){
            texBounds.y += pixelLineHeight;
            continue;
        }

        if (useKerning && prevGlyph) {
            FT_Vector kerning;
            FT_Get_Kerning(face, prevGlyph, glyph, FT_KERNING_DEFAULT, &kerning);
            texBounds.x += kerning.x >> 6;
        }

        if (!FT_Load_Glyph(face, glyph, FT_LOAD_ADVANCE_ONLY)) {
            texBounds.x += face->glyph->metrics.horiAdvance >> 6;
        }

        prevGlyph = glyph;
    }

    // Unlock the font face whilst we reallocate texture
    fontLock.unlock();
    m_bounds = Rectf(vector_static_cast<float>(texBounds));
    m_texture.Reallocate(texBounds, Texture::Format::Format_A8_SRGB);

    // Font textures are graysacle with one bit per pixel
    uint8_t* pixelBuffer = new uint8_t[texBounds.x * texBounds.y];
    // Make sure incase any pixels arent written they are transparent
    memset(pixelBuffer, 0, texBounds.x * texBounds.y);

    // Lock the font again
    fontLock.lock();
    // Set pixel sizes again incase it has been changed by another thread
    if (FT_Set_Pixel_Sizes(face, 0, m_pixelSize)) {
        Arclight::Logger::Error("Failed to set font size!");
        return;
    }

    int xPos = 0;
    int yPos = 0;
    prevGlyph = 0;
    for (unsigned int glyph : glyphs) {
        if(glyph == '\n'){
            yPos += pixelLineHeight;
            continue;
        }

        if (useKerning) {
            if (prevGlyph) {
                FT_Vector kerning;
                FT_Get_Kerning(face, prevGlyph, glyph, FT_KERNING_DEFAULT, &kerning);
                xPos += kerning.x >> 6; // Offset the x position for kerning
            }
        }

        if (FT_Load_Glyph(face, glyph, FT_LOAD_RENDER)) {
            continue;
        }
        FT_GlyphSlot slot = face->glyph;

        // Start of the font blit
        // Ascender is the difference between the baseline and the top of the glyph
        int yOffset = yPos + (face->size->metrics.ascender >> 6) - slot->bitmap_top;
        assert(yOffset >= 0);

        int xOffset = 0;
        if (!useKerning) {
            xOffset = face->glyph->metrics.horiBearingX >> 6;
        }

        // Copy the glyph into the texture
        for (int y = 0; y < slot->bitmap.rows; y++) {
            assert(yOffset + y <= texBounds.y);

            memcpy(&pixelBuffer[(yOffset + y) * texBounds.x + xPos + xOffset],
                   &slot->bitmap.buffer[y * slot->bitmap.pitch], slot->bitmap.width);
        }

        // Advance the x position
        xPos += face->glyph->metrics.horiAdvance >> 6;
        prevGlyph = glyph;
    }
    // No longer need the face
    fontLock.unlock();

    m_vertices[0].position = Vector2f{0, m_bounds.Height()};
    m_vertices[1].position = Vector2f{0, 0};
    m_vertices[2].position = Vector2f{m_bounds.Width(), m_bounds.Height()};
    m_vertices[3].position = Vector2f{m_bounds.Width(), 0};

    // Copy the pixel buffer into the texture
    m_texture.Update(pixelBuffer);
    delete pixelBuffer;
}

void Text::SetFont(std::shared_ptr<Font> font) {
    m_font = std::move(font);

    Render();
}

void Text::SetText(UnicodeString text) {
    m_text = std::move(text);

    Render();
}

void Text::SetFontSize(int pixelSize) {
    m_pixelSize = pixelSize;
    Render();
}

} // namespace Arclight
