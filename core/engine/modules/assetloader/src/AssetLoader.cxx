#include "assetloader/AssetLoader.hxx"
#include <io/IO.hxx>
#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader {
    auto paks = std::vector<std::filesystem::path>();

    void Init() {
        auto currentPath = std::filesystem::current_path();
        auto filelIst = io::GetFileList(currentPath);
        for (const auto& file : filelIst) {
            if (file.is_regular_file() && file.path().extension() == ".pak") {
                paks.push_back(file.path());
            }
        }
    }

    std::vector<uint8_t> TryLoadFile(std::string fileName) {
        std::vector<uint8_t> data;

        for (const auto& pak : paks) {
            if (io::CheckIfFileExists(pak.string(), fileName.data())) {
                data = io::LoadFileFromArchive(pak.string().c_str(), fileName.data());

                return data;
            }
        }

        return {};
    }

    std::string TryFindFile(std::string_view fileName) {
        for (const auto& pak : paks) {
            if (io::CheckIfFileExists(pak.string(), fileName.data())) {
                return pak.string();
            }
        }

        return "";
    }

    std::vector<RawMeshData> LoadMeshes(std::string_view modelName)
    {
        auto data = TryLoadFile(std::string(modelName));

        if (data.empty()) {
            throw std::runtime_error("Failed to load mesh data for model: " + std::string(modelName));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        std::vector<RawMeshData> loadedMeshes;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loadedMeshes);
        }

        return loadedMeshes;
    }

    RawTextureData LoadTexture(std::string_view textureName) {
        auto data = TryLoadFile(std::string(textureName));

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for texture: " + std::string(textureName));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        RawTextureData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawMaterialData LoadMaterial(std::string_view materialName) {
        auto data = TryLoadFile(std::string(materialName));

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for material: " + std::string(materialName));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        RawMaterialData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawShaderData LoadShader(std::string_view shaderName) {
        auto data = TryLoadFile(std::string(shaderName));

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for shader: " + std::string(shaderName));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        RawShaderData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawPhysicsMaterialData LoadPhysicsMaterial(std::string_view name) {
        auto data = TryLoadFile(std::string(name));

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for physics material: " + std::string(name));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        RawPhysicsMaterialData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawCubemapData LoadCubemap(std::string_view name) {
        auto data = TryLoadFile(std::string(name));

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for cubemap: " + std::string(name));
        }

        std::string binaryStr(data.begin(), data.end());

        std::istringstream iss(binaryStr, std::ios::binary);

        RawCubemapData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawAudioData LoadAudio(std::string_view name) {
        auto data = TryLoadFile(std::string(name));
        if (data.empty()) {
            throw std::runtime_error("Failed to load data for audio: " + std::string(name));
        }
        std::string binaryStr(data.begin(), data.end());
        std::istringstream iss(binaryStr, std::ios::binary);
        RawAudioData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }
}
