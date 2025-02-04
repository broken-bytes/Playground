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
        std::vector<uint8_t> Pixels;
        uint32_t Width;
        uint32_t Height;
        uint8_t Channels;

        template <class Archive>
        void serialize(Archive& ar)
        {
            ar(Pixels, Width, Height, Channels);
        }
    };
}
