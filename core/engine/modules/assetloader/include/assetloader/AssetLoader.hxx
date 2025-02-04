#pragma once

#include <cstdint>
#include <vector>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>

namespace playground::assetloader {
    constexpr const char* ASSET_LOADER_VERSION = "01";

    enum class MAGIC_NUMBERS : uint32_t {
        MESH = 0x4D455348,
        TEXTURE = 0x54584554
    };

    const char* MagicNumberStringFor(MAGIC_NUMBERS number) {
        switch (number)
        {
        case playground::assetloader::MAGIC_NUMBERS::MESH:
            return "0x4D455348";
        case playground::assetloader::MAGIC_NUMBERS::TEXTURE:
            return "0x54584554";
        default:
            break;
        }

        return nullptr;
    }

    std::vector<RawMeshData> LoadMeshes(const std::vector<uint8_t>& buffer);
    RawTextureData LoadTextures(const std::vector<uint8_t>& buffer);
}
