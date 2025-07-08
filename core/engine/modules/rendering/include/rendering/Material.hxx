#pragma once

#include "Shader.hxx"
#include <math/Vector2.hxx>
#include <math/Vector3.hxx>
#include <math/Vector4.hxx>
#include <array>
#include <cstdint>
#include <unordered_map>
#include <memory>
#include <string>

namespace playground::rendering {
	class Material {
        struct MaterialBuffer {
            std::array<float, 16> floats;
            std::array<int, 16> ints;
            std::array<math::Vector2, 16> vec2s;
            std::array<math::Vector3, 16> vec3s;
            std::array<math::Vector4, 16> vec4s;
            std::array<uint32_t, 8> textures;
        };

	public:
		Material() {}
	};
}

