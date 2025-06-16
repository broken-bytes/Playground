#include "Math.hxx"
#include <simde/x86/avx2.h>
#include <glm/gtc/type_ptr.hpp>

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
}
