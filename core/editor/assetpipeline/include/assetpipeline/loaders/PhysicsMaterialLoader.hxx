#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>
#include "assetloader/RawPhysicsMaterialData.hxx"

namespace playground::editor::assetpipeline::loaders::physicsmaterialloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawPhysicsMaterialData;
}
