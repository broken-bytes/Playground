#pragma once

#include "math/Vector3.hxx"
#include "math/Vector4.hxx"
#include "math/Quaternion.hxx"
#include "math/Matrix3x3.hxx"
#include "math/Matrix4x4.hxx"
#include <limits>

namespace playground::math {
    template<class T> float ToNormalizedFloat(const T x, double min, double max) {
        return 2 * (x - min) / (max - min) - 1; // note: 0 does not become 0.
    }

    template<class T> float ToNormalizedFloat(const T x) {
        const double valMin = std::numeric_limits<T>::min();
        const double valMax = std::numeric_limits<T>::max();
        return ToNormalizedFloat(x, valMin, valMax);
    }

    void Mat4FromPRS(const Vector3* position, const Quaternion* rotation, const Vector3* scale, Matrix4x4* mat);

    void Mat4FromPRSBulk(const Vector3* position, const Quaternion* rotation, const Vector3* scale, size_t count, Matrix4x4* mats);

    void GetViewMatrix(const Vector3* position, const Quaternion* rotation, Matrix4x4* outMat);

    void LookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up, Matrix4x4* out);

    void GetProjectionMatrix(float fovYDegrees, float aspectRatio, float nearPlane, float farPlane, Matrix4x4* outMat);

    void Transpose(const Matrix3x3& src, Matrix3x3* dst);
    void Transpose(const Matrix4x4& src, Matrix4x4* dst);

    void Inverse(const Matrix3x3& m, Matrix3x3* out);
    void Inverse(const Matrix4x4& m, Matrix4x4* out);


    inline Matrix4x4 Translate(Matrix4x4& mat, const Vector3& translation) {
        Matrix4x4 translationMat = Matrix4x4::Translation(translation.X, translation.Y, translation.Z);

        return mat * translationMat;
    }

    Vector3 Normalize(const Vector3& v);

    inline float RadiansToDegrees(float radians) {
        return radians * (180.0f / 3.14159265358979323846f);
    }

    inline float DegreesToRadians(float degrees) {
        return degrees * (3.14159265358979323846f / 180.0f);
    }

    inline Vector3 operator*(const Quaternion& q, const Vector3& v) {
        // Extract quaternion components
        Vector3 u(q.X, q.Y, q.Z);
        float s = q.W;

        // Compute rotation: v' = 2.0f * dot(u, v) * u
        //                 + (s*s - dot(u, u)) * v
        //                 + 2.0f * s * cross(u, v)
        Vector3 uv = u.Cross(v);
        Vector3 uu = u * u.Dot(v);
        return (uu * 2.0f) + v * (s * s - u.Dot(u)) + uv * (2.0f * s);
    }
}
