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

	inline T Width() { return right - left; }
	inline T Height() { return bottom - top; }

	// Returns whether a point intersects
	inline bool Intersect(Vector2<T> point){
		return (point.x >= left && point.x < right && point.y >= top && point.y < bottom);
	}
};

using Rectf = Rect<float>;

} // namespace Arclight