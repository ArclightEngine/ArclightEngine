#pragma once

#include <memory>
#include <string>
#include <vector>

#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Resource.h>
#include <Arclight/Vector.h>

namespace Arclight {

class Image final : public Resource, NonCopyable {
    ARCLIGHT_OBJECT(Image, Resource)
public:
    Image();

    int Load() override;

    inline const Vector2i& Size() const { return m_size; }
    inline const void* Data() const { return m_pixelData.get(); }

private:
    int LoadImpl();

    std::unique_ptr<uint8_t> m_pixelData; // smart pointer for the pixel data
    Vector2i m_size;                      // Bounds of the image
};

} // namespace Arclight
