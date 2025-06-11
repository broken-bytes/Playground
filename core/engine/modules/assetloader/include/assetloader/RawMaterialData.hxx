#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawMaterialData {
        std::string vertexShaderName;
        std::string pixelShaderName;
        std::map<std::string, float> floats;
        std::map<std::string, int32_t> ints;
        std::map<std::string, std::array<float, 2>> vec2s;
        std::map<std::string, std::array<float, 3>> vec3s;
        std::map<std::string, std::array<float, 4>> vec4s;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(vertexShaderName, pixelShaderName, floats, ints, vec2s, vec3s, vec4s);
        }
    };
}
