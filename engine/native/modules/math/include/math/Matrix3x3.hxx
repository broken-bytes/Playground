#pragma once

#include <array>
#include <string>
#include <cmath>

namespace playground::math {
    struct Matrix4x4;

    struct Matrix3x3 {
        std::array<std::array<float, 3>, 3> elements;

        Matrix3x3();

        Matrix3x3(std::array<float, 9> flatArray) {
            for (int i = 0; i < 3; ++i) {
                for (int j = 0; j < 3; ++j) {
                    elements[i][j] = flatArray[i * 3 + j];
                }
            }
        }

        float& operator()(int row, int col);

        const float& operator()(int row, int col) const;

        const float* Row(int row) const;

        Matrix3x3 operator*(const Matrix3x3& other) const;

        bool operator==(const Matrix3x3& other) const;

        bool operator!=(const Matrix3x3& other) const;

        static Matrix3x3 Identity() {
            return Matrix3x3();
        }

        static Matrix3x3 Zero() {
            return Matrix3x3(std::array<float, 9>{
                0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f,
                    0.0f, 0.0f, 0.0f
            });
        }

        Matrix4x4 ToMatrix4x4() const;

        std::string ToString() const;
    };
}
