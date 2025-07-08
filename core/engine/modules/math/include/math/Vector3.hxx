#pragma once

#include "math/Quaternion.hxx"
#include <string>

namespace playground::math {
    struct Vector3 {
        float X;
        float Y;
        float Z;

        Vector3() : X(0.0f), Y(0.0f), Z(0.0f) {}

        Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

        Vector3 operator+(const Vector3& other) const {
            return Vector3(X + other.X, Y + other.Y, Z + other.Z);
        }

        Vector3 operator-(const Vector3& other) const {
            return Vector3(X - other.X, Y - other.Y, Z - other.Z);
        }

        Vector3 operator-() const {
            return Vector3{ -X, -Y, -Z };
        }

        Vector3 operator*(float scalar) const {
            return Vector3(X * scalar, Y * scalar, Z * scalar);
        }

        Vector3 operator/(float scalar) const {
            if (scalar != 0.0f) {
                return Vector3(X / scalar, Y / scalar, Z / scalar);
            }
            return Vector3(); // Return zero vector if division by zero
        }

        bool operator==(const Vector3& other) const {
            return (X == other.X && Y == other.Y && Z == other.Z);
        }

        bool operator!=(const Vector3& other) const {
            return !(*this == other);
        }

        inline float Length() const {
            return sqrt(X * X + Y * Y + Z * Z);
        }

        inline float Dot(const Vector3& b) const {
            return X * b.X + Y * b.Y + Z * b.Z;
        }

        inline Vector3 Cross(const Vector3& b) const {
            return Vector3(
                Y * b.Z - Z * b.Y,
                Z * b.X - X * b.Z,
                X * b.Y - Y * b.X
            );
        }

        std::string ToString() const {
            return "Vector3(" + std::to_string(X) + ", " + std::to_string(Y) + ", " + std::to_string(Z) + ")";
        }
    };
}
