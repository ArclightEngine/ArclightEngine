#include <Arclight/Graphics/Transform.h>

#include <cmath>
#include <cstdio>

namespace Arclight {

Transform::Transform(const Vector2f& position, const Vector2f& scale, float rotationDegrees)
	: m_position(position), m_scale(scale) {
	m_rotation = rotationDegrees * (M_PI / 180.f);
}

void Transform::SetPosition(const Vector2f& position) {
	m_position = position;

	m_matrixDirty = true;

}
void Transform::SetPosition(float x, float y) {
	return SetPosition({x, y});
}
void Transform::SetScale(const Vector2f& scale) {
	m_scale = scale;

	m_matrixDirty = true;
}
void Transform::SetScale(float scaleX, float scaleY) {
	return SetScale({scaleX, scaleY});
}
void Transform::SetRotation(float degrees) {
	m_rotation = degrees * (M_PIf32 / 180.f);

	m_matrixDirty = true;
}

const Matrix4& Transform::Matrix() {
	if(m_matrixDirty){
		float sin = sinf(m_rotation);
		float cos = cosf(m_rotation);

		m_matrix = Matrix4(
						cos * m_scale.x, -sin * m_scale.y, 0, m_position.x,
						sin * m_scale.x, cos * m_scale.y, 0, m_position.y,
						0,    0,   1, 0,
						0,    0,   0, 1);
	}

	return m_matrix;
}

}; // namespace Arclight