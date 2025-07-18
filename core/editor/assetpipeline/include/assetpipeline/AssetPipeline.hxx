#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>
#include <vector>

#include "assetloader/RawMeshData.hxx"
#include "assetloader/RawMaterialData.hxx"
#include "assetloader/RawTextureData.hxx"
#include "assetloader/RawShaderData.hxx"
#include "assetloader/RawPhysicsMaterialData.hxx"
#include "assetloader/RawCubemapData.hxx"

namespace playground::editor::assetpipeline {
    auto Init() -> void;
    auto CreateDirectory(std::string_view path) -> void;
    auto CreateArchive(const std::filesystem::path& path) -> int8_t;
    auto SaveBufferToArchive(const std::filesystem::path& path, std::string name, std::vector<uint8_t>& buffer) -> int8_t;
    auto CookModel(std::vector<assetloader::RawMeshData> meshData) -> std::vector<uint8_t>;
    auto CookMaterial(assetloader::RawMaterialData materialData) -> std::vector<uint8_t>;
    auto CookTexture(assetloader::RawTextureData textureData) -> std::vector<uint8_t>;
    auto CookShader(std::string code) -> std::vector<uint8_t>;
    auto CookPhysicsMaterial(assetloader::RawPhysicsMaterialData materialData) -> std::vector<uint8_t>;
    auto CookCubemap(assetloader::RawCubemapData cubemapData) -> std::vector<uint8_t>;
}
