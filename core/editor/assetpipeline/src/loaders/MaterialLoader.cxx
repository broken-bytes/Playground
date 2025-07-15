#include "assetpipeline/loaders/MaterialLoader.hxx"
#include <assetloader/RawMaterialData.hxx>
#include <cereal/archives/json.hpp>
#include <cereal/external/rapidjson/rapidjson.h>
#include <filesystem>
#include <fstream>

namespace playground::editor::assetpipeline::loaders::materialloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawMaterialData {
        std::ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open material file: " + path.string());
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

        assetloader::RawMaterialData loadedMaterial;
        {
            try {
                cereal::JSONInputArchive archive(iss);
                archive(
                    cereal::make_nvp("shaderName", loadedMaterial.shaderName),
                    cereal::make_nvp("type", loadedMaterial.type),
                    cereal::make_nvp("textures", loadedMaterial.textures),
                    cereal::make_nvp("cubemaps", loadedMaterial.cubemaps),
                    cereal::make_nvp("props", loadedMaterial.props)
                );
            }
            catch (const cereal::RapidJSONException& e) {
                std::cerr << "Deserialization failed: " << e.what() << std::endl;
            }
        }

        return loadedMaterial;
    }
}
