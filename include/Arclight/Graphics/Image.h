#pragma once

#include <string>
#include <vector>
#include <memory>

#include <Arclight/Vector.h>

namespace Arclight {

class Image {
public:
	Image();

	inline const Vector2i& Size() const { return m_size; }
	inline const void* Data() const { return m_pixelData.get(); }

	int LoadResource(const std::string& name);

private:
	unsigned m_dataSize = 0; // Size of the pixel data buffer
	std::unique_ptr<uint8_t> m_pixelData; // smart pointer for the pixel data
	Vector2i m_size; // Bounds of the image
};

} // namespace Arclight