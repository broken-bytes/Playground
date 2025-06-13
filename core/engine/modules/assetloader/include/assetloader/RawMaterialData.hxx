#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawMaterialData {
        std::string vertexShaderName;
        std::string pixelShaderName;
        //std::map<std::string, float> floats;
        //std::map<std::string, int32_t> ints;
        //std::map<std::string, std::vector<float>> vec2s;
        //std::map<std::string, std::vector<float>> vec3s;
        //std::map<std::string, std::vector<float>> vec4s;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(vertexShaderName),
                CEREAL_NVP(pixelShaderName)
                // CEREAL_NVP(floats),
                // CEREAL_NVP(ints),
                // CEREAL_NVP(vec2s),
                // CEREAL_NVP(vec3s),
                // CEREAL_NVP(vec4s)
            );
        }
    };
}
