#pragma once

#include <cstdint>
#include <filesystem>
#include <tuple>
#include <vector>

#include "assetloader/RawMeshData.hxx"

namespace playground::editor::assetpipeline::loaders::modelloader {
	auto LoadFromFile(
		std::filesystem::path path
	) -> std::vector<assetloader::RawMeshData>;;
}
