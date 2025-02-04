#include "assetloader/AssetLoader.hxx"

#include <stdexcept>

namespace playground::assetloader {

    std::vector<RawMeshData> LoadMeshes(const std::vector<uint8_t>& buffer)
    {
        uint32_t magic, meshCount;
        uint8_t version;
        std::memcpy(&magic, buffer.data(), 4);
        std::memcpy(&version, buffer.data() + 4, 2);
        std::memcpy(&meshCount, buffer.data() + 6, 4);

        if (version != std::stoi(ASSET_LOADER_VERSION)) {
            throw std::runtime_error("Invalid asset loader version");
        }

        if (magic != (uint32_t)MAGIC_NUMBERS::MESH) {
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

    RawTextureData LoadTextures(const std::vector<uint8_t>& buffer) {
        uint32_t magic, mipCount, channels;
        uint8_t version;

        size_t offset = 0;

        std::memcpy(&magic, buffer.data() + offset, 4);
        offset += 4;
        std::memcpy(&version, buffer.data() + offset, 2);
        offset += 2;
        std::memcpy(&channels, buffer.data() + offset, 1);
        offset += 1;

        if (version != std::stoi(ASSET_LOADER_VERSION)) {
            throw std::runtime_error("Invalid asset loader version");
        }

        if (magic != (uint32_t)MAGIC_NUMBERS::TEXTURE) {
            throw std::runtime_error("Invalid texture file");
        }

        uint32_t width, height;

        std::memcpy(&width, buffer.data() + offset, 4);
        offset += 4;
        std::memcpy(&height, buffer.data() + offset, 4);
        offset += 4;

        RawTextureData textureData;
        textureData.Channels = channels;
        textureData.Width = width;
        textureData.Height = height;

        // Copy pixel data

        textureData.Pixels = std::vector<uint8_t>(width * height * channels);

        std::memcpy(&textureData.Pixels, buffer.data() + offset, buffer.size() - offset);

        return textureData;
    }
}
