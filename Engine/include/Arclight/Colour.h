#pragma once

#include <cstdint>

#include <Arclight/Vector.h>

namespace Arclight {

union Colour final {
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	} __attribute__((packed));
	uint32_t value;

	inline constexpr Colour() {}
	inline constexpr Colour(uint32_t _value) : value(_value) {}
	inline constexpr Colour(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b), a(0xff) {}
	inline constexpr Colour(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) : r(_r), g(_g), b(_b), a(_a) {}

	inline constexpr Vector4f AsFloat() const {
		return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}
};

using Color = Colour;
using RGBAColour = Colour;
using RGBAColor = RGBAColour;

} // namespace Arclight