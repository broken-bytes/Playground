#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>
#include "assetloader/RawMeshData.hxx"
#include "assetloader/RawAnimationData.hxx"

namespace playground::editor::assetpipeline::loaders::modelloader {
	auto LoadFromFile(
		std::filesystem::path path
	) -> std::vector<assetloader::RawMeshData>;

    auto LoadAnimationsFromFile(
        std::filesystem::path path
    ) -> std::vector<assetloader::RawAnimationData>;
}
