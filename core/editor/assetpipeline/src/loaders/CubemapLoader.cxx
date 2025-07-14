#include "assetpipeline/loaders/CubemapLoader.hxx"
#include <fstream>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/external/rapidjson/rapidjson.h>
#include <filesystem>

namespace playground::editor::assetpipeline::loaders::cubemaploader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> assetloader::RawCubemapData {
        std::ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open cubemap file: " + path.string());
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

        assetloader::RawCubemapData loadedCubemap;
        {
            try {
                cereal::JSONInputArchive archive(iss);
                archive(
                    cereal::make_nvp("faces", loadedCubemap.faces)
                );
            }
            catch (const cereal::RapidJSONException& e) {
                std::cerr << "Deserialization failed: " << e.what() << std::endl;
            }
        }

        return loadedCubemap;
    }
}
