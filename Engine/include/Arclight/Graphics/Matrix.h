#pragma once

#include <Arclight/Vector.h>

namespace Arclight {

class Matrix4 {
public:
    // Default constructor, iniitalize with identity matrix
    Matrix4();
    // Initialize with matrix, where matrix is an array of 16 float matrices, stored column-major.
    Matrix4(const float* matrix);
    // Iniitialize using the following matrices, row major
    Matrix4(float v00, float v10, float v20, float v30,
                               float v01, float v11, float v21, float v31,
                               float v02, float v12, float v22, float v32,
                               float v03, float v13, float v23, float v33);

    Vector2f Apply(const Vector2f& vector) const;

    // 2D translation
    void Translate(float x, float y);
    inline void Translate(const Vector2f& vector) { return Translate(vector.x, vector.y); }

    // 2D scale
    void Scale(float x, float y);
    inline void Scale(const Vector2f& vector) { return Scale(vector.x, vector.y); }

    // 2D rotation, we only have one axis to rotate around
    void Rotate(float angle);

    inline const float* Matrix() const { return m_matrix; }

    Matrix4& operator*=(const Matrix4& r);
    Matrix4& operator*=(const float* matrix);
    
    static const float s_identityMatrix[16]; // Identity matrix (no transformation)
    
private:
    float m_matrix[16]; // GLSL wants a column-major 4x4 matrix
};

} // namespace Arclight