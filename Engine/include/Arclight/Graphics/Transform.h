#pragma once

#include <Arclight/Graphics/Matrix.h>
#include <Arclight/Vector.h>

#include <mutex>

namespace Arclight {

class Transform {
public:
	Transform(const Vector2f& position = { 0.f, 0.f }, const Vector2f& scale = { 1.f, 1.f }, float rotationDegrees = 0);
	Transform(const Transform&);

	Transform& operator=(const Transform&);

	void SetPosition(const Vector2f& position);
	void SetPosition(float x, float y);
	void SetZIndex(float z);
	void SetScale(const Vector2f& scale);
	void SetScale(float scaleX, float scaleY);
	void SetRotation(float degrees);

	inline const Vector2f& GetPosition() const { return m_position; }
	inline float GetZIndex() const { return m_zIndex; }
	inline const Vector2f& GetScale() const { return m_scale; }

	const Matrix4& Matrix();
private:
	std::mutex m_matrixLock;

	Vector2f m_position;
	float m_zIndex = 0;
	Vector2f m_scale = {1.f, 1.f};
	float m_rotation; // Radians

	bool m_matrixDirty = true; // Only update matrix when necessary, no need to update after every transformation
	Matrix4 m_matrix;
};

} // namespace Arclight