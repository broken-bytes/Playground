#pragma once
#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>
#include "assetloader/RawTextureData.hxx"

namespace playground::editor::assetpipeline::loaders::textureloader {
    auto LoadFromFile(
        std::filesystem::path path,
        bool isNormal
    ) -> assetloader::RawTextureData;
}
