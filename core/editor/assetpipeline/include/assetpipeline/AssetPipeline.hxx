#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

#include "assetloader/RawMeshData.hxx"
#include "assetloader/RawMaterialData.hxx"
#include "assetloader/RawTextureData.hxx"

namespace playground::editor::assetpipeline {
    auto Init() -> void;
    auto CreateDirectory(std::string_view path) -> void;
    auto CreateArchive(const std::filesystem::path& path) -> int8_t;
    auto SaveBufferToArchive(const std::filesystem::path& path, std::string_view name, std::vector<uint8_t>& buffer) -> int8_t;
    auto CookModel(std::vector<assetloader::RawMeshData> meshData) -> std::vector<uint8_t>;
    auto CookMaterial(assetloader::RawMaterialData materialData) -> std::vector<uint8_t>;
    auto CookTexture(assetloader::RawTextureData textureData) -> std::vector<uint8_t>;
}
