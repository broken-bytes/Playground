#pragma once

#include <string>
#include <vector>

namespace playground::assetloader
{
    struct RawTextureData {
        std::vector<uint8_t> Pixels;
        uint32_t Width;
        uint32_t Height;
        uint8_t Channels;
    };
}
