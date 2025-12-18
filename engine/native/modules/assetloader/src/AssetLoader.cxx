#include "assetloader/AssetLoader.hxx"
#include <io/IO.hxx>
#include <shared/Logger.hxx>
#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cstdint>
#include <map>

namespace playground::assetloader {
    auto paks = std::vector<std::filesystem::path>();
    auto mappings = std::map<uint64_t, std::string>();

    std::vector<uint8_t> TryLoadFileFromPak(std::string fileName, std::string pak);
    std::string TryFindFile(std::string_view fileName);

    void Init(const char* path) {
        logging::logger::SetupSubsystem("assetloader");
        logging::logger::Info("Initializing Asset Loader with path " + std::string(path), "assetloader");
        auto filelIst = io::GetFileList(path);
        for (const auto& file : filelIst) {
            if (file.is_regular_file() && file.path().extension() == ".pak") {
                paks.push_back(file.path());

                auto data = TryLoadFileFromPak("_mapping_", file.path().string());

                std::string binaryStr(data.begin(), data.end());

                std::istringstream iss(binaryStr, std::ios::binary);

                std::map<uint64_t, std::string> loadedMeshes;
                {
                    cereal::JSONInputArchive iarchive(iss);
                    iarchive(loadedMeshes);
                }

                for (const auto& [key, value] : loadedMeshes) {
                    mappings[key] = value;
                }
            }
        }
    }

    std::vector<uint8_t> TryLoadFile(uint64_t hash) {
        auto it = mappings.find(hash);
        if (it == mappings.end())
        {
            return {};
        }

        std::vector<uint8_t> data;

        for (const auto& pak : paks) {
            if (io::CheckIfFileExists(pak.string(), it->second.data())) {
                data = io::LoadFileFromArchive(pak.string().c_str(), it->second.data());

                return data;
            }
        }

        return {};
    }

    std::vector<uint8_t> TryLoadFileFromPak(std::string fileName, std::string pak) {
        std::vector<uint8_t> data = io::LoadFileFromArchive(pak.c_str(), fileName.data());
        if (!data.empty())
        {
            return data;
        }

        return {};
    }

    std::string TryFindFile(uint64_t hash) {
        auto it = mappings.find(hash);
        if (it == mappings.end())
        {
            return "";
        }

        for (const auto& pak : paks) {
            if (io::CheckIfFileExists(pak.string(), it->second.data())) {
                return pak.string();
            }
        }

        return "";
    }

    std::vector<RawMeshData> LoadMeshes(uint64_t hash)
    {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load mesh data for model: " + std::to_string(hash));
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

    RawTextureData LoadTexture(uint64_t hash) {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for texture: " + std::to_string(hash));
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

    RawMaterialData LoadMaterial(uint64_t hash) {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for material: " + std::to_string(hash));
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

    RawShaderData LoadShader(uint64_t hash) {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for shader: " + std::to_string(hash));
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

    RawPhysicsMaterialData LoadPhysicsMaterial(uint64_t hash) {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for physics material: " + std::to_string(hash));
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

    RawCubemapData LoadCubemap(uint64_t hash) {
        auto data = TryLoadFile(hash);

        if (data.empty()) {
            throw std::runtime_error("Failed to load data for cubemap: " + std::to_string(hash));
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

    RawAudioData LoadAudio(uint64_t hash) {
        auto data = TryLoadFile(hash);
        if (data.empty()) {
            throw std::runtime_error("Failed to load data for audio: " + std::to_string(hash));
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
