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
            std::array<float, 16> floats; // Reserved -> 0: DeltaTime, 1: SinTime, 2: CosTime, 3: TimeSinceStart 4: Transparency
            std::array<int, 16> ints; // Reserved -> 0: ObjectID, 1: Frame Index
            std::array<glm::vec2, 16> vec2s; // Reserved: 0: UV Scale, 1: UV Offset, 2: Screen Size
            std::array<glm::vec3, 16> vec3s; // Reserved: 0: World Position
            std::array<glm::vec3, 16> vec4s; 
            std::array<uint32_t, 8> textures;
        };

	public:
		Material() {}
	};
}

