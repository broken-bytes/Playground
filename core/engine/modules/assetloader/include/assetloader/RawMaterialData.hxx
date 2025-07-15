#pragma once

#include <array>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>


namespace playground::assetloader
{
    enum class MaterialPropType {
        Float,
        Float2,
        Float3,
        Float4,
        Int,
        Int2,
        Int3,
        Int4,
        UInt,
        Bool
    };

    inline std::string ToLower(std::string str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return str;
    }

    inline MaterialPropType PropFromString(const std::string& s) {
        static const std::unordered_map<std::string, MaterialPropType> map = {
            {"float", MaterialPropType::Float},
            {"float2", MaterialPropType::Float2},
            {"float3", MaterialPropType::Float3},
            {"float4", MaterialPropType::Float4},
            {"int", MaterialPropType::Int},
            {"int2", MaterialPropType::Int2},
            {"int3", MaterialPropType::Int3},
            {"int4", MaterialPropType::Int4},
            {"uint", MaterialPropType::UInt},
            {"bool", MaterialPropType::Bool}
        };

        auto it = map.find(ToLower(s));
        if (it != map.end())
            return it->second;
        throw std::invalid_argument("Invalid MaterialPropType: " + s);
    }

    struct TextureProp {
        std::string name;
        std::string value;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(name),
                CEREAL_NVP(value)
            );
        }
    };

    struct CubemapProp {
        std::string name;
        std::string value;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(name),
                CEREAL_NVP(value)
            );
        }
    };

    struct MaterialProp {
        std::string type;
        std::string name;
        std::string value;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(type),
                CEREAL_NVP(name),
                CEREAL_NVP(value)
            );
        }

        MaterialPropType ActualType() {
            return PropFromString(type);
        }
    };

    struct RawMaterialData {
        std::string shaderName;
        std::string type;
        std::vector<TextureProp> textures;
        std::vector<CubemapProp> cubemaps;
        std::vector<MaterialProp> props;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(shaderName),
                CEREAL_NVP(type),
                CEREAL_NVP(textures),
                CEREAL_NVP(cubemaps),
                CEREAL_NVP(props)
            );
        }
    };

}
