#pragma once

#include "rendering/Texture.hxx"
#include <assetloader/RawTextureData.hxx>
#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace playground::rendering {
    struct TextureUploadJob {
        uint32_t handle;
        assetloader::RawTextureData* rawData;
        void (*callback)(uint32_t, uint32_t);

        TextureUploadJob(uint32_t handle, assetloader::RawTextureData* rawData, void (*callback)(uint32_t, uint32_t))
            : handle(handle), rawData(rawData), callback(callback) {
        }
    };
}
