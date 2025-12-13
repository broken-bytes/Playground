#pragma once

#include "Vector3.hxx"
#include <cmath>

namespace playground::math {
    struct Matrix4x4;

    struct Vector4 {
        float X;
        float Y;
        float Z;
        float W;

        Vector4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
            : X(x), Y(y), Z(z), W(w) {
        }

        Vector4(const Vector4& other)
            : X(other.X), Y(other.Y), Z(other.Z), W(other.W) {
        }

        Vector4(const Vector3& vec3, float w = 1.0f)
            : X(vec3.X), Y(vec3.Y), Z(vec3.Z), W(w) {
        }

        Vector4& operator=(const Vector4& other) {
            if (this != &other) {
                X = other.X;
                Y = other.Y;
                Z = other.Z;
                W = other.W;
            }
            return *this;
        }

        bool operator==(const Vector4& other) const {
            return (X == other.X && Y == other.Y && Z == other.Z && W == other.W);
        }

        bool operator!=(const Vector4& other) const {
            return !(*this == other);
        }

        Vector4 operator+(const Vector4& other) const {
            return Vector4(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
        }

        Vector4 operator-(const Vector4& other) const {
            return Vector4(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
        }

        Vector4 operator*(float scalar) const {
            return Vector4(X * scalar, Y * scalar, Z * scalar, W * scalar);
        }

        Vector4 operator*(const Matrix4x4& m) const;

        Vector4 operator/(float scalar) const {
            if (scalar != 0.0f) {
                return Vector4(X / scalar, Y / scalar, Z / scalar, W / scalar);
            }
            // Handle division by zero appropriately (could throw an exception or return a zero vector)
            return Vector4();
        }

        float Length() const {
            return sqrt(X * X + Y * Y + Z * Z + W * W);
        }
    };
}
