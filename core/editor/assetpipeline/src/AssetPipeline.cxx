#include <fstream>
#include <filesystem>
#include <FreeImagePlus.h>
#include <zip.h>
#include <rendering/Mesh.hxx>
#include <assetloader/AssetLoader.hxx>
#include <assetloader/RawMeshData.hxx>
#include "assetpipeline/assetpipeline.hxx"
#include <sstream>
#include <cereal/cereal.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>


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
        std::ostringstream oss(std::ios::binary);
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(meshData);
        }

        // Convert the output to a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }

    auto CookTexture(assetloader::RawTextureData textureData) -> std::vector<uint8_t> {
        std::ostringstream oss(std::ios::binary);
        {
            cereal::BinaryOutputArchive oarchive(oss);
            oarchive(textureData);
        }

        // Convert the serialised output into a vector<uint8_t>
        std::string outString = oss.str();
        std::vector<uint8_t> buffer(outString.begin(), outString.end());

        return buffer;
    }
}
