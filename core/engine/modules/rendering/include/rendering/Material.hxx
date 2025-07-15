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
    constexpr uint16_t MAX_MATERIAL_SIZE_BYTES = 512;

    enum MaterialType {
        Skybox = 0,
        Standard = 1,
        Shadow = 2
    };

	struct Material {
        uint32_t id = 0;
        MaterialType type = MaterialType::Standard;
        std::vector<uint32_t> textures = {};
        std::vector<uint32_t> cubemaps = {};
        std::vector<float> floats = {};
        std::vector<int> ints = {};
        std::vector<math::Vector2> vec2s = {};
        std::vector<math::Vector3> vec3s = {};
        std::vector<math::Vector4> vec4s = {};
	};
}

