#pragma once

#include "Matrix.h"
#include "Vector.h"

namespace Arclight {

class Transform {
public:
	Transform(const Vector2f& position = { 0.f, 0.f }, const Vector2f& scale = { 1.f, 1.f }, float rotationDegrees = 0);

	void SetPosition(const Vector2f& position);
	void SetPosition(float x, float y);
	void SetScale(const Vector2f& scale);
	void SetScale(float scaleX, float scaleY);
	void SetRotation(float degrees);

	inline const Vector2f& GetPosition() const { return m_position; }
	inline const Vector2f& GetScale() const { return m_scale; }

	const Matrix4& Matrix();
private:
	Vector2f m_position;
	Vector2f m_scale;
	float m_rotation; // Radians

	bool m_matrixDirty = false; // Only update matrix when necessary, no need to update after every transformation
	Matrix4 m_matrix;
};

} // namespace Arclight