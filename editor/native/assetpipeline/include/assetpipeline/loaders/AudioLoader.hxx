#include <assetloader/RawAudioData.hxx>
#include <cereal/archives/json.hpp>
#include <cereal/external/rapidjson/rapidjson.h>
#include <filesystem>
#include <fstream>

namespace playground::editor::assetpipeline::loaders::audioloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::vector<uint8_t> {
        std::ifstream file;
        file.open(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open audio file: " + path.string());
        }

        std::vector<uint8_t> binaryData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        return binaryData;
    }
}
