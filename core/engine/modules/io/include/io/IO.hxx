#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace playground::io {
    auto OpenArchive(std::string path) -> void;
    auto RemoveFile(std::string path) -> void;
    auto GetFileList(std::filesystem::path path) -> std::vector<std::filesystem::directory_entry>;
    auto CheckIfFileExists(std::string path) -> int8_t;
    auto CheckIfFileExists(std::string archivePath, std::string filePath) -> int8_t;
    auto LoadFileFromArchive(
        const char* path,
        const char* name,
        uint8_t** buffer
    ) -> int8_t;
    auto LoadFileFromArchivePartial(
        const char* path, 
        const char* name,
        size_t start,
        size_t count
    ) -> std::vector<uint8_t>;
    auto CreateUUID() -> std::string;
}
