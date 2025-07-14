#pragma once

#include <cstdint>
#include <vector>
#include <assetloader/RawMaterialData.hxx>
#include <assetloader/RawMeshData.hxx>
#include <assetloader/RawTextureData.hxx>
#include <assetloader/RawShaderData.hxx>
#include <assetloader/RawPhysicsMaterialData.hxx>
#include <assetloader/RawCubemapData.hxx>

namespace playground::assetloader {
    constexpr const char* ASSET_LOADER_VERSION = "01";

    enum class MAGIC_NUMBERS : uint32_t {
        MESH = 0x4D455348,
        TEXTURE = 0x54455854,
        MATERIAL = 0x4D41544C,
        SHADER = 0x53484144,
        PHYSICS_MATERIAL = 0x50584D41,
        CUBEMAP = 0x43554245
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
        case playground::assetloader::MAGIC_NUMBERS::PHYSICS_MATERIAL:
            return "0x50584D41";
        case playground::assetloader::MAGIC_NUMBERS::CUBEMAP:
            return "0x43554245";
        default:
            break;
        }

        return nullptr;
    }

    std::vector<RawMeshData> LoadMeshes(std::string_view name);
    RawTextureData LoadTexture(std::string_view name);
    RawMaterialData LoadMaterial(std::string_view name);
    RawShaderData LoadShader(std::string_view shaderName);
    RawPhysicsMaterialData LoadPhysicsMaterial(std::string_view name);
    RawCubemapData LoadCubemap(std::string_view name);
}
