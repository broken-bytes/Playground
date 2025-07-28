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

int main(int argc, char** argv) {
    playground::editor::assetpipeline::Init();

    // 0 -> Working directory
    // 1 -> File Name
    if (argc < 2) {
        std::cerr << "Usage: model2prefab <<filename>> <<output>>" << std::endl;
        return -1;
    }

    auto model = playground::editor::assetpipeline::loaders::modelloader::LoadFromFile(argv[1]);

    std::cout << "Loaded model with " << model.size() << " meshes." << std::endl;

    for (auto& mesh : model) {
        std::cout << "Mesh: " << mesh.name << ", Vertices: " << mesh.vertices.size() << ", Indices: " << mesh.indices.size() << " Pos: " << mesh.posX << std::endl;
    }

    if (model.empty()) {
        std::cerr << "Failed to load model from file: " << argv[1] << std::endl;
        return -1;
    }
}
