#include <Arclight/Graphics/Text.h>

#include <cassert>
#include <stdexcept>

#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Texture.h>

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

FT_Error FreeType::NewFace(Arclight::File* file, FT_Long index, FT_Face* outFace) {
    std::unique_lock lockFT(m_lock);

    std::vector<uint8_t> memoryBuffer;
    size_t fSize = file->GetSize();

    memoryBuffer.resize(fSize);

    if (file->Read(memoryBuffer.data(), fSize) < 0) {
        return FT_Err_Cannot_Open_Resource;
    }

    return FT_New_Memory_Face(m_library, memoryBuffer.data(), fSize, index, outFace);
}

FT_Error FreeType::DoneFace(FT_Face face) {
    std::unique_lock lockFT(m_lock);

    return FT_Done_Face(face);
}

namespace Arclight {

Text::Text() {}
Text::Text(const UnicodeString& text) { SetText(text); }

void Text::SetText(const UnicodeString& text) {
    m_text = text;

    unsigned textWidth = 0;
}

void Text::Draw() {}

} // namespace Arclight
