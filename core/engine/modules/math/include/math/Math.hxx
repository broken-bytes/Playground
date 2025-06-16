#pragma once

#include <glm/glm.hpp>
#include <limits>

namespace playground::math::utils {
    template<class T> float ToNormalizedFloat(const T x, double min, double max) {
        return 2 * (x - min) / (max - min) - 1; // note: 0 does not become 0.
    }

    template<class T> float ToNormalizedFloat(const T x) {
        const double valMin = std::numeric_limits<T>::min();
        const double valMax = std::numeric_limits<T>::max();
        return ToNormalizedFloat(x, valMin, valMax);
    }

    void Mat4FromPRS(const glm::vec3* position, const glm::vec4* rotation, const glm::vec3* scale, glm::mat4* mat);
}
