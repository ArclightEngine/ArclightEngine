#pragma once

#include <cstdint>

namespace Arclight {

struct Colour {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} __attribute__((packed));

} // namespace Arclight