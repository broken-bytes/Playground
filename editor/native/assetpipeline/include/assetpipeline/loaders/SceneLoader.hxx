#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace playground::editor::assetpipeline::loaders::sceneloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::vector<uint8_t>;
}
