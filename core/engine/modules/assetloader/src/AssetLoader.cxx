#include "assetloader/AssetLoader.hxx"

#include <stdexcept>

namespace playground::assetloader {
    constexpr uint32_t MAGIC_NUMBER = 0x4D455348; // "MESH" in hex
    constexpr uint16_t VERSION = 1;

    std::vector<RawMeshData> LoadMeshes(const std::vector<uint8_t>& buffer)
    {
        uint32_t magic, meshCount;
        std::memcpy(&magic, buffer.data(), 4);
        std::memcpy(&meshCount, buffer.data() + 6, 4);

        if (magic != MAGIC_NUMBER) {
            throw std::runtime_error("Invalid model file");
        }

        std::vector<RawMeshData> meshes;

        RawMeshData mesh;
        size_t offset = 10; // Start after header

        for (size_t i = 0; i < meshCount; i++) {
            uint32_t vertexCount, indexCount;
            std::memcpy(&vertexCount, buffer.data() + offset, 4);
            std::memcpy(&indexCount, buffer.data() + offset + 4, 4);
            offset += 8;

            mesh.vertices.resize(vertexCount);
            std::memcpy(mesh.vertices.data(), buffer.data() + offset, vertexCount * sizeof(RawVertex));
            offset += vertexCount * sizeof(RawVertex);

            mesh.indices.resize(indexCount);
            std::memcpy(mesh.indices.data(), buffer.data() + offset, indexCount * sizeof(uint32_t));
            offset += indexCount * sizeof(uint32_t);
            meshes.push_back(mesh);
        }

        return meshes;
    }
}
