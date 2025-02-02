#pragma once

#include <cstdint>
#include <vector>
#include <assetloader/RawMeshData.hxx>

namespace playground::assetloader {
    std::vector<RawMeshData> LoadMeshes(const std::vector<uint8_t>& buffer);
}