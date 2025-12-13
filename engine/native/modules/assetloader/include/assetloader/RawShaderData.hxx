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
    struct RawShaderData {
        enum class ShaderPropertyType : uint8_t {
            Float,
            Int,
            UInt,
            Bool,
            Vec2,
            Vec3,
            Vec4,
            IVec2,
            IVec3,
            IVec4
        };

        struct ShaderProperty {
            uint16_t offset;
            ShaderPropertyType type;

            template <class Archive>
            void serialize(Archive& ar)
            {
                ar(offset, type);
            }
        };

        struct ShaderPropertiesData {
            std::map<std::string, uint16_t> textures;
            std::map<std::string, ShaderProperty> variables;

            template <class Archive>
            void serialize(Archive& ar)
            {
                ar(textures, variables);
            }
        };

        std::string vertexShader;
        std::string pixelShader;
        ShaderPropertiesData properties;
        
        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(vertexShader, pixelShader, properties);
        }
    };
}
