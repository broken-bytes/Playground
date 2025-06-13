#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>
#include "assetloader/RawMaterialData.hxx"

namespace playground::editor::assetpipeline::loaders::materialloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawMaterialData;
}
