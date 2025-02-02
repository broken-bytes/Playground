#include "io/IO.hxx"

#include <sodium.h>
#include <zip.h>
#include <stduuid/uuid.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>
#include <cassert>

namespace playground::io {
	void XOR_Encrypt(std::vector<uint8_t>& data, uint8_t key = 0xAB) {
		for (auto& byte : data) {
			byte ^= key;
		}
	}

	auto OpenArchive(std::string path) -> void {
		zip_t* archive;
		int err;

		if ((archive = zip_open(path.c_str(), ZIP_RDONLY, &err)) == nullptr) {
			zip_error_t error;
			zip_error_init_with_code(&error, err);
			zip_error_fini(&error);

			throw std::runtime_error("Failed to open archive");
		}
	}

	auto RemoveFile(std::string path) -> void {
		std::filesystem::remove(std::filesystem::path(path));
	}

	auto GetFileList(std::filesystem::path path) -> std::vector<std::filesystem::directory_entry> {
		auto files = std::vector<std::filesystem::directory_entry>();

		for (auto& p : std::filesystem::directory_iterator(path)) {
			files.push_back(p);
		}

		return files;
	}

	auto CheckIfFileExists(std::string path) -> int8_t {
		return std::filesystem::exists(std::filesystem::path(path));
	}

	auto CheckIfFileExists(std::string archivePath, std::string filePath) -> int8_t {
		zip_t* archive;
		int err;

		if ((archive = zip_open(archivePath.c_str(), ZIP_RDONLY, &err)) == nullptr) {
			zip_error_t error;
			zip_error_init_with_code(&error, err);
			zip_error_fini(&error);

			return -1;
		}

		zip_stat_t stat;
		zip_stat_init(&stat);
		zip_stat(archive, filePath.c_str(), ZIP_FL_ENC_UTF_8, &stat);
		zip_close(archive);

		return stat.valid & ZIP_STAT_SIZE;
	}

	auto LoadFileFromArchive(
		const char* path,
		const char* name
	) -> std::vector<uint8_t> {
		zip_t* archive;
		int err;

		if ((archive = zip_open(path, ZIP_RDONLY, &err)) == nullptr) {
			zip_error_t error;
			zip_error_init_with_code(&error, err);
			zip_error_fini(&error);
			throw std::runtime_error("Failed to load file");
		}

		zip_stat_t stat;
		zip_stat_init(&stat);
		zip_stat(archive, name, ZIP_FL_ENC_UTF_8, &stat);
		zip_close(archive);

		return LoadFileFromArchivePartial(path, name, 0, stat.size);
	}

	auto LoadFileFromArchivePartial(
		const char* path,
		const char* name,
		size_t start, 
		size_t count
	) -> std::vector<uint8_t> {
		zip_t* archive;
		int err;

		if ((archive = zip_open(path, ZIP_RDONLY, &err)) == nullptr) {
			zip_error_t error;
			zip_error_init_with_code(&error, err);
			zip_error_fini(&error);

			throw std::runtime_error("Failed to load file");
		}

		zip_stat_t stat;
		zip_stat_init(&stat);
		zip_stat(archive, name, ZIP_FL_ENC_UTF_8, &stat);

		zip_file_t* file = zip_fopen(archive, name, ZIP_FL_ENC_UTF_8);
		if (file == nullptr) {
			zip_close(archive);
			throw std::runtime_error("Failed to load file");
		}

		auto rawBuffer = new uint8_t[count];
		auto result = zip_fseek(file, start, SEEK_SET);
		result = zip_file_is_seekable(file);
		result = zip_fread(file, rawBuffer, count);
		result = zip_fclose(file);
		result = zip_close(archive);

		if (result != 0)
		{
			throw std::runtime_error("Failed to load file");
		}

		auto vec = std::vector(rawBuffer, rawBuffer + count);

		XOR_Encrypt(vec);

		return vec;
	}

	auto CreateUUID() -> std::string {
		std::random_device rd;
		auto seed_data = std::array<int, std::mt19937::state_size> {};
		std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
		std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
		std::mt19937 generator(seq);
		uuids::uuid_random_generator gen{ generator };

		uuids::uuid const id = gen();
		assert(!id.is_nil());
		assert(id.as_bytes().size() == 16);
		assert(id.version() == uuids::uuid_version::random_number_based);
		assert(id.variant() == uuids::uuid_variant::rfc);

		return uuids::to_string(id);
	}
}