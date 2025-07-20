#pragma once

#include "math/Matrix4x4.hxx"
#include <cmath>
#include <simde/x86/avx.h>

namespace playground::math {
    struct Vector3;

    struct Quaternion {
        float X;
        float Y;
        float Z;
        float W;

        Quaternion(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 1.0f)
            : X(x), Y(y), Z(z), W(w) {
        }

        Quaternion Inverse() const {
            float normSq = X * X + Y * Y + Z * Z + W * W;
            if (normSq == 0.0f) {
                // Handle zero-length quaternion safely (return identity)
                return { 0.0f, 0.0f, 0.0f, 1.0f };
            }

            float invNorm = 1.0f / normSq;
            return {
                -X * invNorm,
                -Y * invNorm,
                -Z * invNorm,
                 W * invNorm
            };
        }

        Matrix4x4 ToMatrix() const {
            float x = X, y = Y, z = Z, w = W;

            float xx = x * x;
            float yy = y * y;
            float zz = z * z;
            float xy = x * y;
            float xz = x * z;
            float yz = y * z;
            float wx = w * x;
            float wy = w * y;
            float wz = w * z;

            Matrix4x4 mat;

            simde__m128 row0 = simde_mm_setr_ps(
                1.0f - 2.0f * (yy + zz),
                2.0f * (xy + wz),
                2.0f * (xz - wy),
                0.0f
            );

            simde__m128 row1 = simde_mm_setr_ps(
                2.0f * (xy - wz),
                1.0f - 2.0f * (xx + zz),
                2.0f * (yz + wx),
                0.0f
            );

            simde__m128 row2 = simde_mm_setr_ps(
                2.0f * (xz + wy),
                2.0f * (yz - wx),
                1.0f - 2.0f * (xx + yy),
                0.0f
            );

            simde__m128 row3 = simde_mm_setr_ps(0, 0, 0, 1);

            simde_mm_storeu_ps((float*)mat.Row(0), row0);
            simde_mm_storeu_ps((float*)mat.Row(1), row1);
            simde_mm_storeu_ps((float*)mat.Row(2), row2);
            simde_mm_storeu_ps((float*)mat.Row(3), row3);

            return mat;
        }

        Quaternion Normalise();

        Vector3 Forward() const;

        static Quaternion LookRotation(const Vector3& forward, const Vector3& up);

        Quaternion operator*(const Quaternion& other) const {
            return Quaternion(
                W * other.X + X * other.W + Y * other.Z - Z * other.Y,
                W * other.Y - X * other.Z + Y * other.W + Z * other.X,
                W * other.Z + X * other.Y - Y * other.X + Z * other.W,
                W * other.W - X * other.X - Y * other.Y - Z * other.Z
            );
        }

        Quaternion operator+(const Quaternion& other) const {
            return Quaternion(X + other.X, Y + other.Y, Z + other.Z, W + other.W);
        }

        Quaternion operator-(const Quaternion& other) const {
            return Quaternion(X - other.X, Y - other.Y, Z - other.Z, W - other.W);
        }

        Quaternion operator*(float scalar) const {
            return Quaternion(X * scalar, Y * scalar, Z * scalar, W * scalar);
        }

        Quaternion operator/(float scalar) const {
            return Quaternion(X / scalar, Y / scalar, Z / scalar, W / scalar);
        }
    };
}
