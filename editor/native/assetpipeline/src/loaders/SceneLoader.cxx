#include "assetpipeline/loaders/SceneLoader.hxx"

#include <iostream>
#include <fstream>
#include <cereal/archives/json.hpp>
#include <cereal/types/string.hpp>
#include <sstream>
#include <string>

namespace playground::editor::assetpipeline::loaders::sceneloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::vector<uint8_t> {
        std::ifstream file;
        file.open(path, std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open scene file: " + path.string());
        }

        std::vector<uint8_t> binaryData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        return binaryData;
    }
}

