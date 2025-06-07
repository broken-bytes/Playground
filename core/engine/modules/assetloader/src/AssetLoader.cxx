#include "assetloader/AssetLoader.hxx"

#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader {

    std::vector<RawMeshData> LoadMeshes(std::string& modelName)
    {
        std::istringstream iss(modelName, std::ios::binary);

        std::vector<RawMeshData> loadedMeshes;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loadedMeshes);
        }

        return loadedMeshes;
    }

    RawTextureData LoadTexture(std::string& textureName) {
        std::istringstream iss(textureName, std::ios::binary);

        RawTextureData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }
}
