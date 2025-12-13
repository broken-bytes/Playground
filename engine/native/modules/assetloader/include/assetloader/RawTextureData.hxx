#pragma once

#include <string>
#include <vector>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader
{
    struct RawTextureData {
        std::vector<std::vector<uint8_t>> MipMaps;
        uint16_t Width;
        uint16_t Height;
        uint8_t Channels;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(MipMaps, Width, Height, Channels);
        }
    };
}
