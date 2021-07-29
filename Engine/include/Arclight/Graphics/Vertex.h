#pragma once

#include <Arclight/Vector.h>
#include <Arclight/Colour.h>

namespace Arclight {

struct Vertex {
	Vector2f position;
	Vector2f texCoord;
	Vector4f colour = { 1.f, 1.f, 1.f, 1.f }; // Unfortunately vulkan forces us to use 32-bit values in Vertex buffer
};

} // namespace Arclight