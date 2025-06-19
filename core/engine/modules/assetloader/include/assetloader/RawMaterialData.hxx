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
        std::string shaderName;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(
                CEREAL_NVP(shaderName)
            );
        }
    };
}
