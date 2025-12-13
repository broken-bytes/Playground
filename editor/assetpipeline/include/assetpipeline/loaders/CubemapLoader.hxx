#include <cstdint>
#include <filesystem>
#include "assetloader/RawCubemapData.hxx"

namespace playground::editor::assetpipeline::loaders::cubemaploader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawCubemapData;
}
