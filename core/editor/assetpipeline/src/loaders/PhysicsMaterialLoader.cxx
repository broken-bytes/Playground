#include "assetpipeline/loaders/PhysicsMaterialLoader.hxx"
#include <assetloader/RawPhysicsMaterialData.hxx>
#include <cereal/archives/json.hpp>
#include <cereal/external/rapidjson/rapidjson.h>
#include <filesystem>
#include <fstream>

namespace playground::editor::assetpipeline::loaders::physicsmaterialloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawPhysicsMaterialData {
        std::ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open physics material file: " + path.string());
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

        assetloader::RawPhysicsMaterialData loadedMaterial;
        {
            try {
                cereal::JSONInputArchive archive(iss);
                archive(
                    cereal::make_nvp("name", loadedMaterial.name),
                    cereal::make_nvp("staticFriction", loadedMaterial.staticFriction),
                    cereal::make_nvp("dynamicFriction", loadedMaterial.dynamicFriction),
                    cereal::make_nvp("restitution", loadedMaterial.restitution)
                );
            }
            catch (const cereal::RapidJSONException& e) {
                std::cerr << "Deserialization failed: " << e.what() << std::endl;
            }
        }

        return loadedMaterial;
    }
}
