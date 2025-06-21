#pragma once

#include "rendering/Texture.hxx"
#include <assetloader/RawTextureData.hxx>
#include <cstdint>
#include <string>
#include <functional>
#include <vector>

namespace playground::rendering {
    struct TextureUploadJob {
        uint32_t handle;
        assetloader::RawTextureData rawData;
        std::function<void(uint32_t, uint32_t)> callback;
    };
}
