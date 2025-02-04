#include "assetloader/AssetLoader.hxx"

#include <stdexcept>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>

namespace playground::assetloader {

    std::vector<RawMeshData> LoadMeshes(const std::vector<uint8_t>& buffer)
    {
        std::string inString(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        std::istringstream iss(inString, std::ios::binary);

        std::vector<RawMeshData> loadedMeshes;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loadedMeshes);
        }

        return loadedMeshes;
    }

    RawTextureData LoadTexture(const std::vector<uint8_t>& buffer) {
        std::string inString(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        std::istringstream iss(inString, std::ios::binary);

        RawTextureData loaded;
        {
            cereal::BinaryInputArchive iarchive(iss);
            iarchive(loaded);
        }

        return loaded;
    }
}
