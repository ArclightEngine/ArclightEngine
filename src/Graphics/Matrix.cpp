#include <Arclight/Graphics/Matrix.h>

#include <cmath>
#include <cstring>

namespace Arclight {

inline void CombineMatrixes(float* m1, const float* m2){
    // Multiply the first matrix's rows with the second matrix's columns
    float tempMatrix[16] = {
        m1[0] * m2[0] + m1[4] * m2[1] + m1[8] * m2[2] + m1[12] * m2[3],
        m1[1] * m2[0] + m1[5] * m2[1] + m1[9] * m2[2] + m1[13] * m2[3],
        m1[2] * m2[0] + m1[6] * m2[1] + m1[10] * m2[2] + m1[14] * m2[3],
        m1[3] * m2[0] + m1[7] * m2[1] + m1[11] * m2[2] + m1[15] * m2[3],

        m1[0] * m2[4] + m1[4] * m2[5] + m1[8] * m2[6] + m1[12] * m2[7],
        m1[1] * m2[4] + m1[5] * m2[5] + m1[9] * m2[6] + m1[13] * m2[7],
        m1[2] * m2[4] + m1[6] * m2[5] + m1[10] * m2[6] + m1[14] * m2[7],
        m1[3] * m2[4] + m1[7] * m2[5] + m1[11] * m2[6] + m1[15] * m2[7],

        m1[0] * m2[8] + m1[4] * m2[9] + m1[8] * m2[10] + m1[12] * m2[11],
        m1[1] * m2[8] + m1[5] * m2[9] + m1[9] * m2[10] + m1[13] * m2[11],
        m1[2] * m2[8] + m1[6] * m2[9] + m1[10] * m2[10] + m1[14] * m2[11],
        m1[3] * m2[8] + m1[7] * m2[9] + m1[11] * m2[10] + m1[15] * m2[11],

        m1[0] * m2[12] + m1[4] * m2[13] + m1[8] * m2[14] + m1[12] * m2[15],
        m1[1] * m2[12] + m1[5] * m2[13] + m1[9] * m2[14] + m1[13] * m2[15],
        m1[2] * m2[12] + m1[6] * m2[13] + m1[10] * m2[14] + m1[14] * m2[15],
        m1[3] * m2[12] + m1[7] * m2[13] + m1[11] * m2[14] + m1[15] * m2[15],
    };

    memcpy(m1, tempMatrix, 16 * sizeof(float));
}

Matrix4::Matrix4() {
    memcpy(m_matrix, s_identityMatrix, 16 * sizeof(float));
}

Matrix4::Matrix4(const float* matrix){
    memcpy(m_matrix, matrix, 16 * sizeof(float));
}

Matrix4::Matrix4(float v00, float v01, float v02, float v03,
                               float v10, float v11, float v12, float v13,
                               float v20, float v21, float v22, float v23,
                               float v30, float v31, float v32, float v33){
    const float temp[] { // Make sure we store column-major
        v00, v10, v20, v30,
        v01, v11, v21, v31,
        v02, v12, v22, v32, 
        v03, v13, v23, v33,
    };
    memcpy(m_matrix, temp, 16 * sizeof(float));
}

Vector2f Matrix4::Apply(const Vector2f& vector) const {
    return {
        vector.x * m_matrix[0] + vector.x * m_matrix[4] + vector.x * m_matrix[8] + m_matrix[12],
        vector.x * m_matrix[0] + vector.x * m_matrix[4] + vector.x * m_matrix[8] + m_matrix[12],
    };
}

// 2D translation
void Matrix4::Translate(float x, float y){
    // Effectively multiply our matrix with
    // 1, 0, 0, vector.x
    // 0, 1, 0, vector.y
    // 0, 0, 1, 0
    // 0, 0, 0, 1
    const float temp[] {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    };
    CombineMatrixes(m_matrix, temp);
}

// 2D scale
void Matrix4::Scale(float x, float y){
    // Effectively multiply our matrix with
    // vector.x, 0,         0, 0
    // 0,        vector.y,  0, 0
    // 0,        0,         1, 0
    // 0,        0,         0, 1
    const float temp[] {
        x, 0, 0, 0,
        0, y, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    CombineMatrixes(m_matrix, temp);
}

// 2D rotation
void Matrix4::Rotate(float angle){
    float r = angle * (M_PI / 180.f); // Radians
    float c = cosf(r);
    float s = sinf(r);

    // Effectively multiply our matrix with
    // cos, -sin, 0, 0
    // sin, cos,  0, 0
    // 0,   0,    1, 0
    // 0,   0,    0, 1
    const float temp[]{
        c, s, 0, 0,
        -s, c, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    CombineMatrixes(m_matrix, temp);
}

Matrix4& Matrix4::operator*=(const Matrix4& r) {
    CombineMatrixes(m_matrix, r.Matrix());
    return *this;
}

Matrix4& Matrix4::operator*=(const float* otherMatrix) {
    CombineMatrixes(m_matrix, otherMatrix);
    return *this;
}

const float Matrix4::s_identityMatrix[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

} // namespace Arclight