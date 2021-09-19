#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Arclight/Core/Resource.h>
#include <Arclight/NonCopyable.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Image final : public Resource, NonCopyable {
public:
    Image(std::string name);

    int Load() override;

    inline const Vector2i& Size() const { return m_size; }
    inline const void* Data() const { return m_pixelData.get(); }

private:
    int LoadImpl();

    unsigned m_dataSize = 0;              // Size of the pixel data buffer
    std::unique_ptr<uint8_t> m_pixelData; // smart pointer for the pixel data
    Vector2i m_size;                      // Bounds of the image
};

} // namespace Arclight
