#include "Math.hxx"
#include <simde/x86/avx.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

namespace playground::math::utils {
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
}
