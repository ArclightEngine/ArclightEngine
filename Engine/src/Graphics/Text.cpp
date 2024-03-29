#include <Arclight/Graphics/Text.h>

#include <cassert>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Texture.h>

#ifndef NO_ICU
#include <unicode/schriter.h>
#endif

#include "Freetype.h"

FreeType* FreeType::m_instance = new FreeType();

FreeType::FreeType() {
    assert(!m_instance);
    m_instance = this;

    Arclight::Logger::Debug("Initializing Freetype!");

    if (FT_Error e = FT_Init_FreeType(&m_library); e) {
        Arclight::Logger::Error("Error {} initializing freetype!", FT_Error_String(e));
        FatalRuntimeError("Failed to initialize Freetype!");
    }
}

FT_Error FreeType::NewFace(Arclight::File* file, FT_Long index, FT_Face* outFace,
                           std::vector<uint8_t>& outData) {
    std::unique_lock lockFT(m_lock);

    size_t fSize = file->get_size();
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

void Text::render() {
    if (!m_font.get()) {
        return;
    }

    std::unique_lock fontLock(m_font->m_lock);
    FT_Face face = reinterpret_cast<FT_Face>(m_font->m_handle);

    if (FT_Set_Pixel_Sizes(face, 0, m_pixelSize)) {
        Arclight::Logger::Error("Failed to set font size!");
        return;
    }

    bool useKerning = FT_HAS_KERNING(face);
    // In 64ths of a pixel so r shift by 6
    unsigned int pixelLineHeight = face->size->metrics.height >> 6;
    Vector2u texBounds = {0, pixelLineHeight};

    // Compose a vector of glyphs
    std::vector<unsigned int> glyphs;

#ifdef NO_ICU
    for (int codepoint : m_text) {
        if (codepoint == '\n') {
            texBounds.y += pixelLineHeight;
            glyphs.push_back('\n');
        } else if (codepoint != '\r') { // Ignore carriage returns
            glyphs.push_back(FT_Get_Char_Index(face, codepoint));
        }
    }
#else
    icu::StringCharacterIterator it(m_text);
    UChar32 codepoint = it.next32PostInc();
    while (codepoint != icu::StringCharacterIterator::DONE) {
        if (codepoint == '\n') {
            texBounds.y += pixelLineHeight;
            glyphs.push_back('\n');
        } else if (codepoint != '\r') { // Ignore carriage returns
            glyphs.push_back(FT_Get_Char_Index(face, codepoint));
        }

        codepoint = it.next32PostInc();
    }
#endif

    if (glyphs.size() == 0) {
        return;
    }

    unsigned int prevGlyph = 0;
    for (unsigned int glyph : glyphs) {
        if (glyph == '\n') {
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

    // Round up to multiple of 4 bytes
    texBounds.x = (texBounds.x + 3) & (~3U);

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
        if (glyph == '\n') {
            yPos += static_cast<int>(pixelLineHeight);
            xPos = 0;
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
        for (unsigned int y = 0; y < slot->bitmap.rows; y++) {
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

    // Copy the pixel buffer into the texture
    m_texture.Update(pixelBuffer);
    delete pixelBuffer;

    m_vertices[0].position = Vector2f{0, m_bounds.height()};
    m_vertices[0].texCoord = Vector2f{0, 1.f};
    m_vertices[1].position = Vector2f{0, 0};
    m_vertices[1].texCoord = Vector2f{0, 0};
    m_vertices[2].position = Vector2f{m_bounds.width(), m_bounds.height()};
    m_vertices[2].texCoord = Vector2f{1.f, 1.f};
    m_vertices[3].position = Vector2f{m_bounds.width(), 0};
    m_vertices[3].texCoord = Vector2f{1.f, 0};
}

void Text::SetFont(std::shared_ptr<Font> font) {
    m_font = std::move(font);

    render();
}

void Text::SetText(UnicodeString text) {
    m_text = std::move(text);

    render();
}

void Text::SetFontSize(int pixelSize) {
    m_pixelSize = pixelSize;
    render();
}

} // namespace Arclight
