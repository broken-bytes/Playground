#include "assetpipeline/loaders/ShaderLoader.hxx"

#include <fstream>

namespace playground::editor::assetpipeline::loaders::shaderloader {
    auto LoadFromFile(
        std::filesystem::path path
    ) -> std::string {

        std::ifstream file;
        file.open(path);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open material file: " + path.string());
        }

        std::string binaryStr((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        return binaryStr;
    }
}
