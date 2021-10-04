#pragma once

#include <Arclight/Core/Resource.h>
#include <Arclight/Core/NonCopyable.h>

#include <mutex>
#include <vector>

namespace Arclight {

class Font final : public Resource, NonCopyable {
    friend class Text;
    ARCLIGHT_OBJECT(Font, Resource)
public:
    Font();
    ~Font() override;

    int Load() override;

private:
    int LoadImpl();

    // Font data
    std::vector<uint8_t> m_fontData;

    void* m_handle = nullptr; // Font handle, just an abstraction of the freetype object
    std::mutex m_lock; // Font lock, should be acquired when text objects use the FreeType face
};

} // namespace Arclight
