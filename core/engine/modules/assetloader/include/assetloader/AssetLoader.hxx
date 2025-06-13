#pragma once

#include <cstdint>
#include <vector>
#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>
#include <assetloader/RawShaderData.hxx>

namespace playground::assetloader {
    constexpr const char* ASSET_LOADER_VERSION = "01";

    enum class MAGIC_NUMBERS : uint32_t {
        MESH = 0x4D455348,
        TEXTURE = 0x54455854,
        MATERIAL = 0x4D41544C,
        SHADER = 0x53484144
    };

    inline const char* MagicNumberStringFor(MAGIC_NUMBERS number) {
        switch (number)
        {
        case playground::assetloader::MAGIC_NUMBERS::MESH:
            return "0x4D455348";
        case playground::assetloader::MAGIC_NUMBERS::TEXTURE:
            return "0x54455854";
        case playground::assetloader::MAGIC_NUMBERS::MATERIAL:
            return "0x4D41544C";
        case playground::assetloader::MAGIC_NUMBERS::SHADER:
            return "0x53484144";
        default:
            break;
        }

        return nullptr;
    }

    std::vector<RawMeshData> LoadMeshes(std::string_view name);
    RawTextureData LoadTexture(std::string_view name);
    RawMaterialData LoadMaterial(std::string_view name);
    RawShaderData LoadShader(std::string_view shaderName);
}
