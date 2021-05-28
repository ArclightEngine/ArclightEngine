#pragma once

#include <cstdint>

namespace Arclight {

union Colour {
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} __attribute__((packed));
	uint32_t value;
};

using RGBAColour = Colour;

} // namespace Arclight