#pragma once

#include <Arclight/Core/Resource.h>
#include <Arclight/Core/NonCopyable.h>

#include <mutex>

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

    void* m_handle = nullptr; // Font handle, just an abstraction of the freetype object
    std::mutex m_lock; // Font lock, should be acquired when text objects use the FreeType face
};

} // namespace Arclight
