#pragma once

namespace Arclight {

template <typename T>
struct Vector2 {
	T x;
	T y;
};

template <typename T>
inline Vector2<T> operator*(const Vector2<T>& vector, T magnitude){
	return Vector2<T>{ vector.x * magnitude, vector.y * magnitude };
}

template <typename T>
inline bool operator==(const Vector2<T>& l, const Vector2<T>& r){
	return l.x == r.x && l.y == r.y;
}

template <typename T>
inline bool operator!=(const Vector2<T>& l, const Vector2<T>& r){
	return l.x != r.x || l.y != r.y;
}

template <typename T>
inline Vector2<T> operator+(const Vector2<T>& l, const Vector2<T>& r){
	return {l.x + r.x, l.y + r.y};
}

template <typename T>
inline Vector2<T>& operator+=(Vector2<T>& l, const Vector2<T>& r){
	l.x += r.x;
	l.y += r.y;

	return l;
}

template <typename T>
inline Vector2<T> operator-(const Vector2<T>& l, const Vector2<T>& r){
	return {l.x - r.x, l.y - r.y};
}

template <typename T>
inline Vector2<T>& operator-=(Vector2<T>& l, const Vector2<T>& r){
	l.x -= r.x;
	l.y -= r.y;

	return l;
}

template <typename T>
struct Vector3 {
	T x;
	T y;
	T z;
};

template <typename T>
inline bool operator==(const Vector3<T>& l, const Vector3<T>& r){
	return l.x == r.x && l.y == r.y && l.z == r.z;
}

template <typename T>
inline bool operator!=(const Vector3<T>& l, const Vector3<T>& r){
	return l.x != r.x || l.y != r.y || l.z != r.z;
}

template <typename T>
struct Vector4 {
	T x;
	T y;
	T z;
	T w;
};

using Vector2f = Vector2<float>;
using Vector2i = Vector2<int>;
using Vector2u = Vector2<unsigned int>;
using Vector3f = Vector3<float>;
using Vector3i = Vector3<int>;
using Vector3u = Vector3<unsigned int>;
using Vector4f = Vector4<float>;
using Vector4i = Vector4<int>;
using Vector4u = Vector4<unsigned int>;

} // namespace Arclight