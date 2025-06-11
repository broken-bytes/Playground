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
        std::string blob;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(blob);
        }
    };
}
