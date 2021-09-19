#include <Arclight/Graphics/Font.h>

#include <cassert>
#include <stdexcept>

#include <Arclight/Core/File.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/UnicodeString.h>

#include "Freetype.h"

namespace Arclight {

Font::Font(std::string name) : Resource(std::move(name)) {}

Font::~Font() {
    if (m_handle) {
        FT_Error e = FreeType::Instance().DoneFace(reinterpret_cast<FT_Face>(m_handle));
        assert(!e);
    }
}

int Font::Load() { return LoadImpl(); }

int Font::LoadImpl() {
    if (m_handle) {
        FT_Error e = FreeType::Instance().DoneFace(reinterpret_cast<FT_Face>(m_handle));
        assert(!e);
    }

    File* file = File::Open(m_filesystemPath);
    if (!file) {
        Logger::Error("Error opening font face '", m_filesystemPath, "'.");

        m_handle = nullptr;
        return -1;
    }

    FT_Error e = FreeType::Instance().NewFace(file, 0, reinterpret_cast<FT_Face*>(&m_handle));
    delete file;

    if (e) {
        Logger::Error("Error ", e, " loading font face '", m_filesystemPath, "'.");

        m_handle = nullptr;
        return e;
    }

    FT_Face face = reinterpret_cast<FT_Face>(m_handle);
    e = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
    assert(!e);

    assert(m_handle);
    return 0;
}

} // namespace Arclight
