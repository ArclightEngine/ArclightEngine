#pragma once

#include <Arclight/Core/Resource.h>

#include <mutex>

namespace Arclight {

class Font final : public Resource {
    friend class Text;
public:
    Font(std::string name);
    ~Font() override;

    int Load() override;

private:
    int LoadImpl();

    void* m_handle = nullptr; // Font handle, just an abstraction of the freetype object
    std::mutex m_lock; // Font lock, should be acquired when text objects use the FreeType face
};

} // namespace Arclight
