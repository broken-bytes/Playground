#include "assetloader/AssetLoader.hxx"

#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader {

    std::vector<RawMeshData> LoadMeshes(std::string_view modelName)
    {
        std::istringstream iss(std::string(modelName), std::ios::binary);

        std::vector<RawMeshData> loadedMeshes;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loadedMeshes);
        }

        return loadedMeshes;
    }

    RawTextureData LoadTexture(std::string_view textureName) {
        std::istringstream iss(std::string(textureName), std::ios::binary);

        RawTextureData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawMaterialData LoadMaterial(std::string_view materialName) {
        std::istringstream iss(std::string(materialName), std::ios::binary);

        RawMaterialData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }

    RawShaderData LoadShader(std::string_view shaderName) {
        std::istringstream iss(std::string(shaderName), std::ios::binary);

        RawShaderData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }
}
