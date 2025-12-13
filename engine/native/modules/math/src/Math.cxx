#include "math/Math.hxx"
#include "math/Vector3.hxx"
#include "math/Matrix4x4.hxx"
#include "math/Quaternion.hxx"
#include <cmath>
#include <simde/x86/avx.h>
#include <iostream>

namespace playground::math {
    math::Matrix4x4 Mat4FromPRS(const Vector3& position, const Quaternion& rotation, const Vector3& scale) {
        Matrix4x4 mat;
        Mat4FromPRS(&position, &rotation, &scale, &mat);

        return mat;
    }

    void Mat4FromPRS(const Vector3* position, const Quaternion* rotation, const Vector3* scale, Matrix4x4* mat) {
        const float x = rotation->X;
        const float y = rotation->Y;
        const float z = rotation->Z;
        const float w = rotation->W;

        const float xx = x * x, yy = y * y, zz = z * z;
        const float xy = x * y, xz = x * z, yz = y * z;
        const float wx = w * x, wy = w * y, wz = w * z;

        // Compute rotation matrix rows (row-major order)
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

        // Apply scale
        simde__m128 scaleVec = simde_mm_setr_ps(scale->X, scale->Y, scale->Z, 1.0f);
        row0 = simde_mm_mul_ps(row0, scaleVec);
        row1 = simde_mm_mul_ps(row1, scaleVec);
        row2 = simde_mm_mul_ps(row2, scaleVec);

        // Final translation row
        simde__m128 row3 = simde_mm_setr_ps(position->X, position->Y, position->Z, 1.0f);

        // Store to Matrix4x4
        Matrix4x4 result;
        simde_mm_storeu_ps((float*)result.Row(0), row0);
        simde_mm_storeu_ps((float*)result.Row(1), row1);
        simde_mm_storeu_ps((float*)result.Row(2), row2);
        simde_mm_storeu_ps((float*)result.Row(3), row3);

        *mat = result;
    }

    void Mat4FromPRSBulk(const Vector3* position, const Quaternion* rotation, const Vector3* scale, size_t count, Matrix4x4* mats) {
        for(size_t i = 0; i < count; ++i) {
            Mat4FromPRS(&position[i], &rotation[i], &scale[i], &mats[i]);
        }
    }

    void GetViewMatrix(const Vector3* position, const Quaternion* rotation, Matrix4x4* outMat) {
        // First compute the rotation matrix from the *normalised* quaternion (no inverse)
        const float x = rotation->X;
        const float y = rotation->Y;
        const float z = rotation->Z;
        const float w = rotation->W;

        const float r00 = 1.0f - 2.0f * (y * y + z * z);
        const float r01 = 2.0f * (x * y - z * w);
        const float r02 = 2.0f * (x * z + y * w);

        const float r10 = 2.0f * (x * y + z * w);
        const float r11 = 1.0f - 2.0f * (x * x + z * z);
        const float r12 = 2.0f * (y * z - x * w);

        const float r20 = 2.0f * (x * z - y * w);
        const float r21 = 2.0f * (y * z + x * w);
        const float r22 = 1.0f - 2.0f * (x * x + y * y);

        // Now apply negative position using this rotation (this is the view matrix math)
        const float px = position->X;
        const float py = position->Y;
        const float pz = position->Z;

        const float tx = -(r00 * px + r01 * py + r02 * pz);
        const float ty = -(r10 * px + r11 * py + r12 * pz);
        const float tz = -(r20 * px + r21 * py + r22 * pz);

        *outMat = Matrix4x4({
            r00, r01, r02, tx,
            r10, r11, r12, ty,
            r20, r21, r22, tz,
            0.0f, 0.0f, 0.0f, 1.0f
        });
    }

    void LookAtLH(const Vector3& eye, const Vector3& target, const Vector3& up, Matrix4x4* out) {
        Vector3 zAxis = (target - eye).Normalise();        // Forward
        Vector3 xAxis = up.Cross(zAxis).Normalise();       // Right
        Vector3 yAxis = zAxis.Cross(xAxis);                // Up

        std::array<float, 16> flat = {
            xAxis.X, yAxis.X, zAxis.X, 0.0f,
            xAxis.Y, yAxis.Y, zAxis.Y, 0.0f,
            xAxis.Z, yAxis.Z, zAxis.Z, 0.0f,
            -xAxis.Dot(eye), -yAxis.Dot(eye), -zAxis.Dot(eye), 1.0f
        };

        *out = Matrix4x4(flat);
    }

    void OrthographicLH(float left, float right, float bottom, float top, float nearZ, float farZ, Matrix4x4* out) {
        float rml = right - left;
        float tmb = top - bottom;
        float fmn = farZ - nearZ;

        *out = Matrix4x4({
            2.0f / rml, 0.0f,       0.0f,           -(right + left) / rml,
            0.0f,       2.0f / tmb, 0.0f,           -(top + bottom) / tmb,
            0.0f,       0.0f,       1.0f / fmn,     -nearZ / fmn,
            0.0f,       0.0f,       0.0f,           1.0f
            });
    }

    void GetProjectionMatrix(float fovYDegrees, float aspectRatio, float nearPlane, float farPlane, Matrix4x4* outMat) {
        float fovYRadians = DegreesToRadians(fovYDegrees);
        float yScale = 1.0f / tanf(fovYRadians * 0.5f);
        float xScale = yScale / aspectRatio;

        float n = nearPlane;
        float f = farPlane;

        Matrix4x4 matrix({
            xScale, 0.0f,    0.0f,                      0.0f,
            0.0f,   yScale,  0.0f,                      0.0f,
            0.0f,   0.0f,    f / (f - n),               1.0f,
            0.0f,   0.0f,   -n * f / (f - n),           0.0f
        });

        *outMat = matrix;
    }

    void Transpose(const Matrix3x3& src, Matrix3x3* dst) {
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                (*dst)(col, row) = src(row, col);
            }
        }
    }

    void Transpose(const Matrix4x4& src, Matrix4x4* dst) {
        simde__m128 row0 = simde_mm_loadu_ps((simde_float32*)src.Row(0));
        simde__m128 row1 = simde_mm_loadu_ps((simde_float32*)src.Row(1));
        simde__m128 row2 = simde_mm_loadu_ps((simde_float32*)src.Row(2));
        simde__m128 row3 = simde_mm_loadu_ps((simde_float32*)src.Row(3));

        simde__m128 tmp0 = simde_mm_unpacklo_ps(row0, row1);
        simde__m128 tmp1 = simde_mm_unpackhi_ps(row0, row1);
        simde__m128 tmp2 = simde_mm_unpacklo_ps(row2, row3);
        simde__m128 tmp3 = simde_mm_unpackhi_ps(row2, row3);

        simde_mm_storeu_ps((simde_float32*)dst->Row(0), simde_mm_movelh_ps(tmp0, tmp2));
        simde_mm_storeu_ps((simde_float32*)dst->Row(1), simde_mm_movehl_ps(tmp2, tmp0));
        simde_mm_storeu_ps((simde_float32*)dst->Row(2), simde_mm_movelh_ps(tmp1, tmp3));
        simde_mm_storeu_ps((simde_float32*)dst->Row(3), simde_mm_movehl_ps(tmp3, tmp1));
    }

    void Inverse(const Matrix3x3& m, Matrix3x3* out) {
        float a00 = m(0, 0), a01 = m(0, 1), a02 = m(0, 2);
        float a10 = m(1, 0), a11 = m(1, 1), a12 = m(1, 2);
        float a20 = m(2, 0), a21 = m(2, 1), a22 = m(2, 2);

        float det = a00 * (a11 * a22 - a12 * a21)
            - a01 * (a10 * a22 - a12 * a20)
            + a02 * (a10 * a21 - a11 * a20);

        if (fabsf(det) < 1e-6f) {
            *out = Matrix3x3::Identity(); // or handle singular matrix
            return;
        }

        float invDet = 1.0f / det;

        *out = Matrix3x3({
             (a11 * a22 - a12 * a21) * invDet,
            -(a01 * a22 - a02 * a21) * invDet,
             (a01 * a12 - a02 * a11) * invDet,

            -(a10 * a22 - a12 * a20) * invDet,
             (a00 * a22 - a02 * a20) * invDet,
            -(a00 * a12 - a02 * a10) * invDet,

             (a10 * a21 - a11 * a20) * invDet,
            -(a00 * a21 - a01 * a20) * invDet,
             (a00 * a11 - a01 * a10) * invDet,
            });
    }

    void Inverse(const Matrix4x4& m, Matrix4x4* out) {
        // Load rows
        simde__m128 row0 = simde_mm_loadu_ps((simde_float32*)m.Row(0));
        simde__m128 row1 = simde_mm_loadu_ps((simde_float32*)m.Row(1));
        simde__m128 row2 = simde_mm_loadu_ps((simde_float32*)m.Row(2));
        simde__m128 row3 = simde_mm_loadu_ps((simde_float32*)m.Row(3));

        // Transpose to get columns in row registers
        simde__m128 tmp0 = simde_mm_shuffle_ps(row0, row1, 0x44);
        simde__m128 tmp1 = simde_mm_shuffle_ps(row0, row1, 0xEE);
        simde__m128 tmp2 = simde_mm_shuffle_ps(row2, row3, 0x44);
        simde__m128 tmp3 = simde_mm_shuffle_ps(row2, row3, 0xEE);

        simde__m128 r0 = simde_mm_shuffle_ps(tmp0, tmp2, 0x88);
        simde__m128 r1 = simde_mm_shuffle_ps(tmp0, tmp2, 0xDD);
        simde__m128 r2 = simde_mm_shuffle_ps(tmp1, tmp3, 0x88);
        simde__m128 r3 = simde_mm_shuffle_ps(tmp1, tmp3, 0xDD);

        // Compute minors
        simde__m128 tmp4 = simde_mm_mul_ps(r2, r3);
        simde__m128 tmp5 = simde_mm_shuffle_ps(tmp4, tmp4, 0xB1);

        simde__m128 minor0 = simde_mm_sub_ps(
            simde_mm_mul_ps(r1, tmp5),
            simde_mm_shuffle_ps(simde_mm_mul_ps(r1, tmp5), simde_mm_mul_ps(r1, tmp5), 0x4E)
        );

        simde__m128 minor1 = simde_mm_sub_ps(
            simde_mm_mul_ps(r0, tmp5),
            simde_mm_shuffle_ps(simde_mm_mul_ps(r0, tmp5), simde_mm_mul_ps(r0, tmp5), 0x4E)
        );

        tmp4 = simde_mm_mul_ps(r1, r2);
        tmp5 = simde_mm_shuffle_ps(tmp4, tmp4, 0xB1);

        simde__m128 minor2 = simde_mm_sub_ps(
            simde_mm_mul_ps(r3, tmp5),
            simde_mm_shuffle_ps(simde_mm_mul_ps(r3, tmp5), simde_mm_mul_ps(r3, tmp5), 0x4E)
        );

        simde__m128 minor3 = simde_mm_sub_ps(
            simde_mm_mul_ps(r0, tmp5),
            simde_mm_shuffle_ps(simde_mm_mul_ps(r0, tmp5), simde_mm_mul_ps(r0, tmp5), 0x4E)
        );

        // Compute determinant
        simde__m128 det = simde_mm_mul_ps(r0, minor0);
        det = simde_mm_add_ps(det, simde_mm_shuffle_ps(det, det, 0x4E));
        det = simde_mm_add_ps(det, simde_mm_shuffle_ps(det, det, 0xB1));

        // Extract scalar determinant for check
        float detScalar;
        simde_mm_store_ss(&detScalar, det);

        if (fabsf(detScalar) < 1e-6f) {
            *out = Matrix4x4::Identity(); // fallback to identity or error
            return;
        }

        // Reciprocal determinant
        simde__m128 rDet = simde_mm_div_ps(simde_mm_set1_ps(1.0f), det);

        // Scale adjugate by reciprocal determinant
        minor0 = simde_mm_mul_ps(minor0, rDet);
        minor1 = simde_mm_mul_ps(minor1, rDet);
        minor2 = simde_mm_mul_ps(minor2, rDet);
        minor3 = simde_mm_mul_ps(minor3, rDet);

        // Store transposed (row-major) result
        simde_mm_storeu_ps((simde_float32*)out->Row(0), minor0);
        simde_mm_storeu_ps((simde_float32*)out->Row(1), minor1);
        simde_mm_storeu_ps((simde_float32*)out->Row(2), minor2);
        simde_mm_storeu_ps((simde_float32*)out->Row(3), minor3);
    }

    void InverseAffine(const Matrix4x4& m, Matrix4x4* out) {
        // Transpose the upper-left 3x3 part (rotation)
        float r00 = m.elements[0][0], r01 = m.elements[1][0], r02 = m.elements[2][0];
        float r10 = m.elements[0][1], r11 = m.elements[1][1], r12 = m.elements[2][1];
        float r20 = m.elements[0][2], r21 = m.elements[1][2], r22 = m.elements[2][2];

        float tx = m.elements[0][3];
        float ty = m.elements[1][3];
        float tz = m.elements[2][3];

        // Transposed rotation matrix
        out->elements[0][0] = r00;
        out->elements[0][1] = r10;
        out->elements[0][2] = r20;

        out->elements[1][0] = r01;
        out->elements[1][1] = r11;
        out->elements[1][2] = r21;

        out->elements[2][0] = r02;
        out->elements[2][1] = r12;
        out->elements[2][2] = r22;

        // Compute new translation: -R^T * T
        out->elements[0][3] = -(r00 * tx + r10 * ty + r20 * tz);
        out->elements[1][3] = -(r01 * tx + r11 * ty + r21 * tz);
        out->elements[2][3] = -(r02 * tx + r12 * ty + r22 * tz);

        // Last row
        out->elements[3][0] = 0.0f;
        out->elements[3][1] = 0.0f;
        out->elements[3][2] = 0.0f;
        out->elements[3][3] = 1.0f;
    }
}
