#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace playground::io {
    struct FileHandle {
        void* archive = nullptr;
        void* file = nullptr;
        size_t fileSize = 0;
        size_t headerSize = 0;
        size_t readOffset = 0;
        char* archivePath = nullptr;
        char* fileName = nullptr;
    };

    auto OpenArchive(std::string path) -> void;
    auto RemoveFile(std::string path) -> void;
    auto GetFileList(std::filesystem::path path) -> std::vector<std::filesystem::directory_entry>;
    auto CheckIfFileExists(std::string path) -> int8_t;
    auto CheckIfFileExists(std::string archivePath, std::string filePath) -> int8_t;
    FileHandle* OpenFileFromArchive(
        const char* path,
        const char* name
    );
    std::vector<uint8_t> ReadFileFromArchive(
        FileHandle* handle,
        size_t length
    );
    void SeekFileInArchive(
        FileHandle* handle,
        size_t offset
    );
    void CloseFile(FileHandle* handle);
    auto LoadFileFromArchive(
        const char* path,
        const char* name
    ) -> std::vector<uint8_t>;
    auto LoadFileFromArchivePartial(
        const char* path, 
        const char* name,
        size_t start,
        size_t count
    ) -> std::vector<uint8_t>;
    size_t GetFileSizeInArchive(std::filesystem::path path, std::string filePath);
    auto CreateUUID() -> std::string;
}
