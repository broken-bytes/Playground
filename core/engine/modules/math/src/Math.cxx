#include "Math.hxx"
#include <simde/x86/avx.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace playground::math::utils {
    // Note: Use vec4 instead of vec3 for position and scale due to padding on the Swift side.
    void Mat4FromPRS(const glm::vec3* position, const glm::vec4* rotation, const glm::vec3* scale, glm::mat4* mat) {
        const float x = rotation->x;
        const float y = rotation->y;
        const float z = rotation->z;
        const float w = rotation->w;

        // Quaternion to rotation matrix (column-major, GL-style)
        const float xx = x * x, yy = y * y, zz = z * z;
        const float xy = x * y, xz = x * z, yz = y * z;
        const float wx = w * x, wy = w * y, wz = w * z;

        // Unscaled rotation matrix, rows as SIMD vectors
        simde__m128 row0 = simde_mm_set_ps(0.0f,
            1.0f - 2.0f * (yy + zz),
            2.0f * (xy + wz),
            2.0f * (xz - wy)
        );

        simde__m128 row1 = simde_mm_set_ps(0.0f,
            2.0f * (xy - wz),
            1.0f - 2.0f * (xx + zz),
            2.0f * (yz + wx)
        );

        simde__m128 row2 = simde_mm_set_ps(0.0f,
            2.0f * (xz + wy),
            2.0f * (yz - wx),
            1.0f - 2.0f * (xx + yy)
        );

        // Apply scale (element-wise)
        simde__m128 scaleVec = simde_mm_set_ps(0.0f, scale->z, scale->y, scale->x);
        row0 = simde_mm_mul_ps(row0, scaleVec);
        row1 = simde_mm_mul_ps(row1, scaleVec);
        row2 = simde_mm_mul_ps(row2, scaleVec);

        // Final row (position)
        simde__m128 row3 = simde_mm_set_ps(1.0f, position->z, position->y, position->x);

        // Convert to glm::mat4
        glm::mat4 result;
        simde_mm_storeu_ps(glm::value_ptr(result[0]), row0);
        simde_mm_storeu_ps(glm::value_ptr(result[1]), row1);
        simde_mm_storeu_ps(glm::value_ptr(result[2]), row2);
        simde_mm_storeu_ps(glm::value_ptr(result[3]), row3);

        *mat = result;
    }

    void Mat4FromPRSBulk(const glm::vec3* position, const glm::vec4* rotation, const glm::vec3* scale, size_t count, glm::mat4* mats) {
        for(size_t i = 0; i < count; ++i) {
            Mat4FromPRS(&position[i], &rotation[i], &scale[i], &mats[i]);
        }
    }

    void GetViewMatrixSIMD_LH(const glm::vec3* position, const glm::vec4* rotation, glm::mat4* outMat) {
        const float x = rotation->x, y = rotation->y, z = rotation->z, w = rotation->w;

        const float xx = x * x, yy = y * y, zz = z * z;
        const float xy = x * y, xz = x * z, yz = y * z;
        const float wx = w * x, wy = w * y, wz = w * z;

        // Left-handed rotation matrix (no transpose)
        simde__m128 row0 = simde_mm_set_ps(0.0f,
            1.0f - 2.0f * (yy + zz),
            2.0f * (xy + wz),
            -2.0f * (xz - wy) // Flip Z
        );

        simde__m128 row1 = simde_mm_set_ps(0.0f,
            2.0f * (xy - wz),
            1.0f - 2.0f * (xx + zz),
            -2.0f * (yz + wx) // Flip Z
        );

        simde__m128 row2 = simde_mm_set_ps(0.0f,
            2.0f * (xz + wy),
            2.0f * (yz - wx),
            1.0f - 2.0f * (xx + yy)
        );

        // Negative translation
        simde__m128 negPos = simde_mm_set_ps(0.0f, -position->z, -position->y, -position->x);

        // Compute dot products for translation
        simde__m128 t0 = simde_mm_mul_ps(row0, negPos);
        simde__m128 t1 = simde_mm_mul_ps(row1, negPos);
        simde__m128 t2 = simde_mm_mul_ps(row2, negPos);

        float tx = simde_mm_cvtss_f32(simde_mm_hadd_ps(simde_mm_hadd_ps(t0, t0), t0));
        float ty = simde_mm_cvtss_f32(simde_mm_hadd_ps(simde_mm_hadd_ps(t1, t1), t1));
        float tz = simde_mm_cvtss_f32(simde_mm_hadd_ps(simde_mm_hadd_ps(t2, t2), t2));

        simde__m128 row3 = simde_mm_set_ps(1.0f, tz, ty, tx);

        glm::mat4 view;
        simde_mm_storeu_ps(glm::value_ptr(view[0]), row0);
        simde_mm_storeu_ps(glm::value_ptr(view[1]), row1);
        simde_mm_storeu_ps(glm::value_ptr(view[2]), row2);
        simde_mm_storeu_ps(glm::value_ptr(view[3]), row3);

        *outMat = view;
    }

    void GetProjectionMatrixSIMD_LH(float fovYDegrees, float aspectRatio, float nearPlane, float farPlane, glm::mat4* outMat) {
        float fovYRadians = glm::radians(fovYDegrees);
        float f = 1.0f / tanf(fovYRadians / 2.0f);
        float nf = 1.0f / (farPlane - nearPlane);

        float m00 = f / aspectRatio;
        float m11 = f;
        float m22 = farPlane * nf;
        float m32 = -nearPlane * farPlane * nf;

        simde__m128 col0 = simde_mm_set_ps(0.0f, 0.0f, 0.0f, m00);
        simde__m128 col1 = simde_mm_set_ps(0.0f, 0.0f, m11, 0.0f);
        simde__m128 col2 = simde_mm_set_ps(1.0f, 0.0f, m22, 0.0f);   // Z = +1 in clip space
        simde__m128 col3 = simde_mm_set_ps(0.0f, 0.0f, m32, 0.0f);

        glm::mat4 proj;
        simde_mm_storeu_ps(glm::value_ptr(proj[0]), col0);
        simde_mm_storeu_ps(glm::value_ptr(proj[1]), col1);
        simde_mm_storeu_ps(glm::value_ptr(proj[2]), col2);
        simde_mm_storeu_ps(glm::value_ptr(proj[3]), col3);

        *outMat = proj;
    }

    void Transpose(const glm::mat4& src, glm::mat4* dst) {
        simde__m128 row0 = simde_mm_loadu_ps(glm::value_ptr(src[0]));
        simde__m128 row1 = simde_mm_loadu_ps(glm::value_ptr(src[1]));
        simde__m128 row2 = simde_mm_loadu_ps(glm::value_ptr(src[2]));
        simde__m128 row3 = simde_mm_loadu_ps(glm::value_ptr(src[3]));

        simde__m128 tmp0 = simde_mm_unpacklo_ps(row0, row1);
        simde__m128 tmp1 = simde_mm_unpackhi_ps(row0, row1);
        simde__m128 tmp2 = simde_mm_unpacklo_ps(row2, row3);
        simde__m128 tmp3 = simde_mm_unpackhi_ps(row2, row3);

        simde_mm_storeu_ps(glm::value_ptr((*dst)[0]), simde_mm_movelh_ps(tmp0, tmp2));
        simde_mm_storeu_ps(glm::value_ptr((*dst)[1]), simde_mm_movehl_ps(tmp2, tmp0));
        simde_mm_storeu_ps(glm::value_ptr((*dst)[2]), simde_mm_movelh_ps(tmp1, tmp3));
        simde_mm_storeu_ps(glm::value_ptr((*dst)[3]), simde_mm_movehl_ps(tmp3, tmp1));
    }

    void Inverse(const glm::mat4& m, glm::mat4* out) {
        simde__m128 row0 = simde_mm_loadu_ps(glm::value_ptr(m[0]));
        simde__m128 row1 = simde_mm_loadu_ps(glm::value_ptr(m[1]));
        simde__m128 row2 = simde_mm_loadu_ps(glm::value_ptr(m[2]));
        simde__m128 row3 = simde_mm_loadu_ps(glm::value_ptr(m[3]));

        simde__m128 tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7, tmp8, tmp9, tmp10, tmp11;
        simde__m128 minor0, minor1, minor2, minor3;

        tmp0 = simde_mm_shuffle_ps(row0, row1, 0x44); // [0 1] [4 5]
        tmp1 = simde_mm_shuffle_ps(row0, row1, 0xEE); // [2 3] [6 7]
        tmp2 = simde_mm_shuffle_ps(row2, row3, 0x44);
        tmp3 = simde_mm_shuffle_ps(row2, row3, 0xEE);

        simde__m128 r0 = simde_mm_shuffle_ps(tmp0, tmp2, 0x88);
        simde__m128 r1 = simde_mm_shuffle_ps(tmp0, tmp2, 0xDD);
        simde__m128 r2 = simde_mm_shuffle_ps(tmp1, tmp3, 0x88);
        simde__m128 r3 = simde_mm_shuffle_ps(tmp1, tmp3, 0xDD);

        // Now r0, r1, r2, r3 are the transposed rows of the matrix

        tmp0 = simde_mm_mul_ps(r2, r3);
        tmp1 = simde_mm_shuffle_ps(tmp0, tmp0, 0xB1); // Shuffle for cross products

        tmp2 = simde_mm_mul_ps(r1, tmp1);
        tmp3 = simde_mm_mul_ps(r0, tmp1);
        tmp2 = simde_mm_sub_ps(tmp2, simde_mm_shuffle_ps(tmp2, tmp2, 0x4E));
        tmp3 = simde_mm_sub_ps(tmp3, simde_mm_shuffle_ps(tmp3, tmp3, 0x4E));

        minor0 = tmp2;
        minor1 = tmp3;

        tmp0 = simde_mm_mul_ps(r1, r2);
        tmp1 = simde_mm_shuffle_ps(tmp0, tmp0, 0xB1);

        tmp2 = simde_mm_mul_ps(r3, tmp1);
        tmp3 = simde_mm_mul_ps(r0, tmp1);
        tmp2 = simde_mm_sub_ps(tmp2, simde_mm_shuffle_ps(tmp2, tmp2, 0x4E));
        tmp3 = simde_mm_sub_ps(tmp3, simde_mm_shuffle_ps(tmp3, tmp3, 0x4E));

        minor2 = tmp2;
        minor3 = tmp3;

        simde__m128 det = simde_mm_mul_ps(r0, minor0);
        det = simde_mm_add_ps(det, simde_mm_shuffle_ps(det, det, 0x4E));
        det = simde_mm_add_ps(det, simde_mm_shuffle_ps(det, det, 0xB1));

        simde__m128 rDet = simde_mm_div_ps(simde_mm_set1_ps(1.0f), det);

        minor0 = simde_mm_mul_ps(minor0, rDet);
        minor1 = simde_mm_mul_ps(minor1, rDet);
        minor2 = simde_mm_mul_ps(minor2, rDet);
        minor3 = simde_mm_mul_ps(minor3, rDet);

        simde_mm_storeu_ps(glm::value_ptr((*out)[0]), minor0);
        simde_mm_storeu_ps(glm::value_ptr((*out)[1]), minor1);
        simde_mm_storeu_ps(glm::value_ptr((*out)[2]), minor2);
        simde_mm_storeu_ps(glm::value_ptr((*out)[3]), minor3);
    }
}
