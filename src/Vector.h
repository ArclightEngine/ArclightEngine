#pragma once

template <typename T>
struct Vector2 {
	T x;
	T y;
};

using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;