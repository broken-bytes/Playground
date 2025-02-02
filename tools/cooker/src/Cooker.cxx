#include <iostream>
#include <assetpipeline/AssetPipeline.hxx>
#include <assetpipeline/loaders/ModelLoader.hxx>


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

        playground::editor::assetpipeline::SaveBufferToArchive(argv[4], argv[2], buffer);
    }
}
