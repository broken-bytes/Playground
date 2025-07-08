#pragma once

#include <array>
#include <string>

namespace playground::math {
    struct Matrix3x3;

    struct Matrix4x4 {
        std::array<float, 16> elements;

        Matrix4x4();

        Matrix4x4(const std::array<float, 16>& elements);

        Matrix4x4 operator*(const Matrix4x4& other) const;

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

        static Matrix4x4 Zero() {
            return Matrix4x4(std::array<float, 16>{0.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 0.0f});
        }

        static Matrix4x4 Translation(float x, float y, float z) {
            return Matrix4x4(std::array<float, 16>{1.0f, 0.0f, 0.0f, x,
                                                   0.0f, 1.0f, 0.0f, y,
                                                   0.0f, 0.0f, 1.0f, z,
                                                   0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 Scaling(float sx, float sy, float sz) {
            return Matrix4x4(std::array<float, 16>{sx, 0.0f, 0.0f, 0.0f,
                                                   0.0f, sy, 0.0f, 0.0f,
                                                   0.0f, 0.0f, sz, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 RotationX(float angle) {
            float c = cos(angle);
            float s = sin(angle);
            return Matrix4x4(std::array<float, 16>{1.0f, 0.0f, 0.0f, 0.0f,
                                                   0.0f, c, -s, 0.0f,
                                                   0.0f, s, c, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 RotationY(float angle) {
            float c = cos(angle);
            float s = sin(angle);
            return Matrix4x4(std::array<float, 16>{c, 0.0f, s, 0.0f,
                                                   0.0f, 1.0f, 0.0f, 0.0f,
                                                   -s, 0.0f, c, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 RotationZ(float angle) {
            float c = cos(angle);
            float s = sin(angle);
            return Matrix4x4(std::array<float, 16>{c, -s, 0.0f, 0.0f,
                                                   s, c, 0.0f, 0.0f,
                                                   0.0f, 0.0f, 1.0f, 0.0f,
                                                   0.0f, 0.0f, 0.0f, 1.0f});
        }

        static Matrix4x4 Perspective(float fov, float aspect, float nearPlane, float farPlane) {
            float tanHalfFov = tan(fov / 2.0f);
            return Matrix4x4(std::array<float, 16>{
                1.0f / (aspect * tanHalfFov), 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f / tanHalfFov, 0.0f, 0.0f,
                0.0f, 0.0f, -(farPlane + nearPlane) / (farPlane - nearPlane), -1.0f,
                0.0f, 0.0f, -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane), 0.0f
            });
        }

        static Matrix4x4 Orthographic(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
            return Matrix4x4(std::array<float, 16>{
                2.0f / (right - left), 0.0f, 0.0f, -(right + left) / (right - left),
                0.0f, 2.0f / (top - bottom), 0.0f, -(top + bottom) / (top - bottom),
                0.0f, 0.0f, -2.0f / (farPlane - nearPlane), -(farPlane + nearPlane) / (farPlane - nearPlane),
                0.0f, 0.0f, 0.0f, 1.0f
            });
        }

        std::string ToString() const;

        Matrix3x3 ToMatrix3x3() const;
    };
}
