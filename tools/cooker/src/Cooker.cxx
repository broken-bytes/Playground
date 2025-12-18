#define NOMINMAX
#include <iostream>
#include <assetpipeline/AssetPipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <assetpipeline/loaders/MaterialLoader.hxx>
#include <assetpipeline/loaders/TextureLoader.hxx>
#include <assetpipeline/loaders/ShaderLoader.hxx>
#include <assetpipeline/loaders/PhysicsMaterialLoader.hxx>
#include <assetpipeline/loaders/CubemapLoader.hxx>
#include <assetpipeline/loaders/AudioLoader.hxx>
#include <cereal/cereal.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <fstream>

#include "../../../engine/native/modules/shared/include/shared/Hasher.hxx"
#include "assetloader/AssetLoader.hxx"
#include "assetpipeline/loaders/SceneLoader.hxx"

struct CookerAsset {
    std::string location;
    std::string type;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(
            CEREAL_NVP(location),
            CEREAL_NVP(type)
        );
    }
};

struct CookerConfig {
    int maxSize; // The max size per pak file in MB
    std::vector<CookerAsset> assets;

    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(
            CEREAL_NVP(maxSize),
            CEREAL_NVP(assets)
        );
    }
};

int main(int argc, char** argv) {
    playground::editor::assetpipeline::Init();

    std::ifstream file;
    file.open(std::filesystem::path(argv[1]) / "cooker.json", std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not find cooker conig in directory");
    }

    std::string binaryStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::cout << "JSON:\n" << binaryStr << std::endl;

    std::string& s = binaryStr;
    if (s.size() >= 3 &&
        static_cast<uint8_t>(s[0]) == 0xEF &&
        static_cast<uint8_t>(s[1]) == 0xBB &&
        static_cast<uint8_t>(s[2]) == 0xBF) {
        std::cout << "⚠️ BOM detected — removing\n";
        s.erase(0, 3);
    }

    std::istringstream iss(binaryStr);

    CookerConfig config;
    {
        try {
            cereal::JSONInputArchive archive(iss);
            archive(
                cereal::make_nvp("maxSize", config.maxSize),
                cereal::make_nvp("assets", config.assets)
            );
        }
        catch (const cereal::RapidJSONException& e) {
            std::cerr << "Deserialization failed: " << e.what() << std::endl;
            exit(1);
        }
    }

    uint32_t bytes = 0;
    uint16_t currentPackIndex = 1;

    std::stringstream pakName;
    pakName << "content_package_";
    pakName << currentPackIndex;
    pakName << ".pak";
    auto pakPath = std::filesystem::path(std::string(argv[1])) / pakName.str();

    auto mappings = playground::assetloader::AssetMappingsFile();

    for (auto asset : config.assets) {
        std::vector<uint8_t> buffer;
        std::string ending = "";

        auto filePath = std::filesystem::path(std::string(argv[1])) / asset.location;

        if (std::strcmp(asset.type.c_str(), "Model") == 0) {
            auto meshes = playground::editor::assetpipeline::loaders::modelloader::LoadFromFile(filePath);
            buffer = playground::editor::assetpipeline::CookModel(meshes);

            auto animations = playground::editor::assetpipeline::loaders::modelloader::LoadAnimationsFromFile(filePath);

            ending = ".mod";
            // TODO: Write animations
        }
        else if (std::strcmp(asset.type.c_str(), "Texture") == 0) {
            auto texture = playground::editor::assetpipeline::loaders::textureloader::LoadFromFile(filePath, false);
            buffer = playground::editor::assetpipeline::CookTexture(texture);

            ending = ".tex";
        }
        else if (std::strcmp(asset.type.c_str(), "Normal") == 0) {
            auto texture = playground::editor::assetpipeline::loaders::textureloader::LoadFromFile(filePath, true);
            buffer = playground::editor::assetpipeline::CookTexture(texture);

            ending = ".bump";
        }
        else if (std::strcmp(asset.type.c_str(), "Material") == 0) {
            auto material = playground::editor::assetpipeline::loaders::materialloader::LoadFromFile(filePath);
            buffer = playground::editor::assetpipeline::CookMaterial(material);

            ending = ".mat";
        }
        else if (std::strcmp(asset.type.c_str(), "Shader") == 0) {
            auto shader = playground::editor::assetpipeline::loaders::shaderloader::LoadFromFile(filePath);
            buffer = playground::editor::assetpipeline::CookShader(shader);

            ending = ".shader";
        }
        else if (std::strcmp(asset.type.c_str(), "PhysicsMaterial") == 0) {
            auto physicsMaterial = playground::editor::assetpipeline::loaders::physicsmaterialloader::LoadFromFile(filePath);
            buffer = playground::editor::assetpipeline::CookPhysicsMaterial(physicsMaterial);

            ending = ".pmat";
        }
        else if (std::strcmp(asset.type.c_str(), "Audio") == 0) {
            auto audio = playground::editor::assetpipeline::loaders::audioloader::LoadFromFile(filePath);
            buffer = audio;

            ending = ".audio";
        }
        else if (std::strcmp(asset.type.c_str(), "Scene") == 0) {
            auto scene = playground::editor::assetpipeline::loaders::sceneloader::LoadFromFile(filePath);
            buffer = scene;

            ending = ".scene";
        }
        else if (std::strcmp(asset.type.c_str(), "Cubemap") == 0) {
            auto cubemap = playground::editor::assetpipeline::loaders::cubemaploader::LoadFromFile(filePath);

            std::vector<playground::assetloader::RawTextureData> faces(6);

            for(int x = 0; x < 6; x++) {
                auto texture = playground::editor::assetpipeline::loaders::textureloader::LoadFromFile(filePath.parent_path() / cubemap.faces[x], false);

                if (x == 0) {
                    cubemap.Width = texture.Width;
                    cubemap.Height = texture.Height;
                    cubemap.Channels = texture.Channels;
                }

                if (x > 0) {
                    if (cubemap.Width != texture.Width || cubemap.Height != texture.Height || cubemap.Channels != texture.Channels) {
                        std::cout << "Cubemap face " << x << " does not match previous face dimensions. All faces must have the same dimensions." << std::endl;
                        return -1;
                    }
                }

                faces[x] = texture;
            }

            cubemap.facesData = faces;

            buffer = playground::editor::assetpipeline::CookCubemap(cubemap);

            ending = ".cube";
        }
        else {
            std::cout << "Unknown asset type: " << asset.type.c_str() << std::endl;
            return -1;
        }

        // Remove the extension from the asset location if it is not an audio file (audio files have .strings.bank which would break)
        if (ending != ".audio") {
            size_t pos = asset.location.find(".");
            if (pos != std::string::npos) {
                asset.location.erase(pos);
            }
        }
        else {
            // For audio files, we keep the full location as it may contain .strings.bank
            asset.location = asset.location.substr(0, asset.location.find_last_of('.'));
        }

        auto vfsPath = asset.location + ending;

        if ((bytes + buffer.size()) <= config.maxSize * 1024 * 1024) {
            playground::editor::assetpipeline::SaveBufferToArchive(pakPath, vfsPath, buffer, ending != ".audio");
            bytes += buffer.size();
        }
        else {
            currentPackIndex++;
            playground::editor::assetpipeline::SaveBufferToArchive(pakPath, vfsPath, buffer, ending != ".audio");
            bytes = buffer.size();
        }

        mappings.Mappings.insert({playground::shared::Hash(vfsPath), vfsPath});
    }

    auto buffer = playground::editor::assetpipeline::CookMappings(mappings.Mappings);

    playground::editor::assetpipeline::SaveBufferToArchive(pakPath, "_mapping_", buffer, false);
}
