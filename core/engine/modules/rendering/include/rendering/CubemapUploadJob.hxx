#pragma once

#include "rendering/Cubemap.hxx"
#include <assetloader/RawCubemapData.hxx>
#include <cstdint>
#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace playground::rendering {
    struct CubemapUploadJob {
        uint32_t handle;
        std::shared_ptr<assetloader::RawCubemapData> cubemapData;
        std::function<void(uint32_t, uint32_t)> callback;

        ~CubemapUploadJob() {
            std::cout << "CubemapUploadJob destroyed for handle: " << handle << std::endl;
        }
    };
}
