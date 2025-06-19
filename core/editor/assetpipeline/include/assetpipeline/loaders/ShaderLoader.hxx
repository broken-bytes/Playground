#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>
#include "assetloader/RawShaderData.hxx"

namespace playground::editor::assetpipeline::loaders::shaderloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::string;
}
