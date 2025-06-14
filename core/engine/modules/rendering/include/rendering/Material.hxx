#pragma once

#include "Shader.hxx"
#include <glm/glm.hpp>
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
            std::array<glm::vec2, 16> vec2s;
            std::array<glm::vec3, 16> vec3s;
            std::array<glm::vec3, 16> vec4s; 
            std::array<uint32_t, 8> textures;
        };

	public:
		Material() {}
	};
}

