#pragma once

#include <Arclight/Vector.h>

namespace Arclight {

template<typename T>
struct Rect {
	union{
		Vector2<T> origin;
		struct {
			T left; // x1
			T top; // y1
		};
	};
	union {
		Vector2<T> end;
		struct {
			T right; // x2
			T bottom; // y2
		};
	};

	Rect() = default;
	Rect(const Vector2<T>& origin, const Vector2<T>& end) : origin(origin), end(end) {}
	Rect(const Vector2<T>& size)
		: origin(0, 0), end(size) {}

	inline T width() { return right - left; }
	inline T height() { return bottom - top; }

	// Returns whether a point intersects
	inline bool intersect(const Vector2<T>& point){
		return (point.x >= left && point.x < right && point.y >= top && point.y < bottom);
	}
};

template<typename T>
ALWAYS_INLINE Rect<T> operator+(const Vector2<T>& l, const Rect<T>& r) {
	return Rect(l + r.origin, l + r.end);
}

template<typename T>
ALWAYS_INLINE Rect<T> operator+(const Rect<T>& r, const Vector2<T>& l) {
	return Rect(l + r.origin, l + r.end);
}

using Rectf = Rect<float>;

} // namespace Arclight
