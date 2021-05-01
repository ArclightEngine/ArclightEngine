#pragma once

#include "Vector.h"
#include "Colour.h"

namespace Arclight::Graphics {

struct Vertex {
	Vector2i position;
	Colour colour;
};

} // namespace Graphics