#define NOMINMAX
#include <iostream>
#include <assetpipeline/AssetPipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>
#include <assetpipeline/loaders/MaterialLoader.hxx>
#include <assetpipeline/loaders/TextureLoader.hxx>
#include <assetpipeline/loaders/ShaderLoader.hxx>

int main(int argc, char** argv) {
    // 1: Asset Path
    // 2: Asset Name
    // 3: Asset Type
    // 4: Asset Ouput Path

    playground::editor::assetpipeline::Init();

    for (int i = 0; i < argc; i++) {
        std::cout << argv[i] << std::endl;
    }

    if (argc < 5) {
        std::cout << "Usage: <Asset Path> <Asset Name> <Asset Type> <Asset Output Path>" << std::endl;
        return -1;
    }

    std::vector<uint8_t> buffer;

    if (std::strcmp(argv[3], "Model") == 0) {
        auto meshes = playground::editor::assetpipeline::loaders::modelloader::LoadFromFile(argv[1]);
        buffer = playground::editor::assetpipeline::CookModel(meshes);

        auto animations = playground::editor::assetpipeline::loaders::modelloader::LoadAnimationsFromFile(argv[1]);

        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    }
    else if (std::strcmp(argv[3], "Texture") == 0) {
        auto texture = playground::editor::assetpipeline::loaders::textureloader::LoadFromFile(argv[1], false);
        buffer = playground::editor::assetpipeline::CookTexture(texture);

        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    } else if (std::strcmp(argv[3], "Normal") == 0) {
        auto texture = playground::editor::assetpipeline::loaders::textureloader::LoadFromFile(argv[1], true);
        buffer = playground::editor::assetpipeline::CookTexture(texture);

        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    } else if (std::strcmp(argv[3], "Material") == 0) {
        auto material = playground::editor::assetpipeline::loaders::materialloader::LoadFromFile(argv[1]);
        buffer = playground::editor::assetpipeline::CookMaterial(material);
        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    }
    else if (std::strcmp(argv[3], "Shader") == 0) {
        auto shader = playground::editor::assetpipeline::loaders::shaderloader::LoadFromFile(argv[1]);
        buffer = playground::editor::assetpipeline::CookShader(shader);
        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    }
    else {
        std::cout << "Unknown asset type: " << argv[3] << std::endl;
        return -1;
    }
}
