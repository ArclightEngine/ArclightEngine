#pragma once

#include <Arclight/Platform/Platform.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef ARCLIGHT_PLATFORM_WASM
#define FT_Error_String(s) ((const char*)(""))
#endif

// Private wrapper class for FreeType

#include <cassert>
#include <mutex>
#include <vector>

class FreeType {
public:
    FreeType();

    static inline FreeType& Instance() {
        assert(m_instance);
        return *m_instance;
    }

    // Thread-safe wrapper functions for FreeType
    FT_Error NewFace(class Arclight::File* file, FT_Long index, FT_Face* outFace, std::vector<uint8_t>& outData);
    FT_Error DoneFace(FT_Face face);

private:
    static FreeType* m_instance;

    std::mutex m_lock;
    FT_Library m_library = nullptr;
};
