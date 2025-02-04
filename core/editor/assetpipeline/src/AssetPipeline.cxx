#include <fstream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <zip.h>
#include <rendering/Mesh.hxx>
#include <assetloader/AssetLoader.hxx>
#include <assetloader/RawMeshData.hxx>
#include "assetpipeline/assetpipeline.hxx"

using namespace playground::assetloader;

namespace playground::editor::assetpipeline {
    void XOR_Encrypt(std::vector<uint8_t>& data, uint8_t key = 0xAB) {
        for (auto& byte : data) {
            byte ^= key;
        }
    }

    auto Init() -> void
    {
    }

    auto CreateDirectory(const std::string_view path) -> void {
        create_directory(std::filesystem::path(path));
    }

    auto CreateArchive(const std::filesystem::path& path) -> int8_t {
        zip_t* archive;
        int err;

        if ((archive = zip_open(path.string().c_str(), ZIP_CREATE, &err)) == nullptr) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            zip_error_fini(&error);

            return -1;
        }

        zip_close(archive);

        return 0;
    }

    auto SaveBufferToArchive(const std::filesystem::path& path, const std::string_view name, std::vector<uint8_t>& buffer) -> int8_t {
        XOR_Encrypt(buffer);

        zip_t* archive;
        int err;

        if ((archive = zip_open(path.string().c_str(), ZIP_CREATE, &err)) == nullptr) {
            zip_error_t error;
            zip_error_init_with_code(&error, err);
            zip_error_fini(&error);

            return -1;
        }

        zip_source_t* s = nullptr;
        if ((s = zip_source_buffer(archive, buffer.data(), buffer.size(), 0)) == nullptr ||
            zip_file_add(archive, name.data(), s, ZIP_FL_OVERWRITE | ZIP_FL_ENC_UTF_8) < 0) {
            zip_source_free(s);
        }

        zip_close(archive);

        return 0;
    }

    auto CookModel(std::vector<assetloader::RawMeshData> meshData) -> std::vector<uint8_t> {
        // Load the mesh
        std::vector<uint8_t> buffer;

        auto magic = assetloader::MagicNumberStringFor(MAGIC_NUMBERS::MESH);

        // Write header
        uint32_t meshCount = meshData.size();
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(magic), reinterpret_cast<const uint8_t*>(magic) + sizeof(magic));
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(ASSET_LOADER_VERSION), reinterpret_cast<const uint8_t*>(ASSET_LOADER_VERSION) + 2);
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&meshCount), reinterpret_cast<uint8_t*>(&meshCount) + 4);

        for (auto& mesh : meshData) {
            uint32_t vertexCount = mesh.vertices.size();
            uint32_t indexCount = mesh.indices.size();

            buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&vertexCount), reinterpret_cast<uint8_t*>(&vertexCount) + 4);
            buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&indexCount), reinterpret_cast<uint8_t*>(&indexCount) + 4);

            // Write vertices
            for (auto& vertex : mesh.vertices) {
                buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&vertex), reinterpret_cast<uint8_t*>(&vertex) + sizeof(rendering::Vertex));
            }

            // Write indices
            buffer.insert(
                buffer.end(),
                reinterpret_cast<uint8_t*>(mesh.indices.data()),
                reinterpret_cast<uint8_t*>(mesh.indices.data()) + mesh.indices.size() * sizeof(uint32_t)
            );
        }

        return buffer;
    }

    auto CookTexture(assetloader::RawTextureData textureData) -> std::vector<uint8_t> {
        std::vector<uint8_t> buffer;

        auto magic = assetloader::MagicNumberStringFor(MAGIC_NUMBERS::TEXTURE);

        // Write header
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(magic), reinterpret_cast<const uint8_t*>(magic) + sizeof(magic));
        buffer.insert(buffer.end(), reinterpret_cast<const uint8_t*>(ASSET_LOADER_VERSION), reinterpret_cast<const uint8_t*>(ASSET_LOADER_VERSION) + 2);
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&textureData.Channels), reinterpret_cast<uint8_t*>(&textureData.Channels) + 4);

        uint32_t width = textureData.Width;
        uint32_t height = textureData.Height;

        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&width), reinterpret_cast<uint8_t*>(&width) + 4);
        buffer.insert(buffer.end(), reinterpret_cast<uint8_t*>(&height), reinterpret_cast<uint8_t*>(&height) + 4);

        for (auto& pixel : textureData.Pixels) {
            buffer.push_back(pixel);
        }

        return buffer;
    }
}
