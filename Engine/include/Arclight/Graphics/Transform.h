#pragma once

#include <Arclight/Graphics/Matrix.h>
#include <Arclight/Vector.h>

#include <mutex>

namespace Arclight {

class Transform2D {
public:
	Transform2D(const Vector2f& position = { 0.f, 0.f }, const Vector2f& scale = { 1.f, 1.f }, float rotationDegrees = 0);
	Transform2D(const Transform2D&);

	Transform2D& operator=(const Transform2D&);

	void set_position(const Vector2f& position);
	void set_position(float x, float y);
	void set_z_index(float z);
	void set_scale(const Vector2f& scale);
	void set_scale(float scaleX, float scaleY);
	void set_rotation(float degrees);

	inline const Vector2f& get_position() const { return m_position; }
	inline float get_z_index() const { return m_zIndex; }
	inline const Vector2f& get_scale() const { return m_scale; }

	const Matrix4& matrix();
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