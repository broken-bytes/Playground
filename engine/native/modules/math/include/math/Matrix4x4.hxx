#pragma once

#include <array>
#include <string>

namespace playground::math {
    struct Matrix3x3;
    struct Vector4;

    struct Matrix4x4 {
        std::array<std::array<float, 4>, 4> elements;

        Matrix4x4();

        Matrix4x4(std::array<float, 16> flatArray) {
            for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 4; ++j) {
                    elements[i][j] = flatArray[i * 4 + j];
                }
            }
        }

        Matrix4x4 operator*(const Matrix4x4& other) const;

        Vector4 operator*(const Vector4& v) const;

        Matrix4x4& operator*=(const Matrix4x4& other);

        Matrix4x4 operator+(const Matrix4x4& other) const;

        Matrix4x4& operator+=(const Matrix4x4& other);

        Matrix4x4 operator-(const Matrix4x4& other) const;

        Matrix4x4& operator-=(const Matrix4x4& other);

        bool operator==(const Matrix4x4& other) const;

        bool operator!=(const Matrix4x4& other) const;

        float& operator()(int row, int col);

        const float& operator()(int row, int col) const;

        const float* Row(int row) const;

        static Matrix4x4 Identity() {
            return Matrix4x4();
        }

        static Matrix4x4 Zero();
        static Matrix4x4 Translation(float x, float y, float z);
        static Matrix4x4 Scaling(float sx, float sy, float sz);
        static Matrix4x4 RotationX(float angle);
        static Matrix4x4 RotationY(float angle);
        static Matrix4x4 RotationZ(float angle);
        static Matrix4x4 Perspective(float fov, float aspect, float nearPlane, float farPlane);
        static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane);

        static Matrix4x4 OrthographicOffCenter(float left, float right, float bottom, float top, float nearZ, float farZ)
        {
            Matrix4x4 result = {};  // zero-initialize all elements

            float rml = right - left;
            float tmb = top - bottom;
            float fmn = farZ - nearZ;

            result.elements[0][0] = 2.0f / rml;
            result.elements[1][1] = 2.0f / tmb;
            result.elements[2][2] = 1.0f / fmn;

            result.elements[0][3] = -(right + left) / rml;
            result.elements[1][3] = -(top + bottom) / tmb;
            result.elements[2][3] = -nearZ / fmn;

            result.elements[3][3] = 1.0f;

            return result;
        }

        std::string ToString() const;

        Matrix3x3 ToMatrix3x3() const;
    };
}
