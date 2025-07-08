#pragma once

#include <cmath>

namespace playground::math {
    struct Vector2 {
        float X;
        float Y;

        Vector2(float x = 0.0f, float y = 0.0f) : X(x), Y(y) {}

        Vector2 operator+(const Vector2& other) const {
            return Vector2(X + other.X, Y + other.Y);
        }

        Vector2 operator-(const Vector2& other) const {
            return Vector2(X - other.X, Y - other.Y);
        }

        Vector2 operator*(float scalar) const {
            return Vector2(X * scalar, Y * scalar);
        }

        Vector2 operator/(float scalar) const {
            return Vector2(X / scalar, Y / scalar);
        }

        float Length() const {
            return std::sqrt(X * X + Y * Y);
        }
    };
}
