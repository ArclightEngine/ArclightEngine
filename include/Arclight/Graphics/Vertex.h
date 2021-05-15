#pragma once

#include <Arclight/Vector.h>
#include <Arclight/Colour.h>

namespace Arclight {

struct Vertex {
	Vector2f position;
	Vector4f colour; // Unfortunately vulkan forces us to use 32-bit values in Vertex buffer
};

} // namespace Arclight