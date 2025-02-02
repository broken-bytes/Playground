#include "assetdatabase/AssetDatabase.hxx"

#include <io/IO.hxx>

#include <sqlite3.h>

namespace playground::editor::assetdatabase {
	void* database;

	/**
	* @brief Creates or loads an asset database.
	* @param path The path of the database.
	* @return The database.
	*/
	void* CreateOrLoad(const std::filesystem::path& path);
	void Close();

	std::string StringFromType(AssetType type) {
		switch (type)
		{
			case AssetType::Texture:
				return "Texture";
			case AssetType::Model:
				return "Model";
			case AssetType::Audio:
				return "Audio";
			case AssetType::Prefab:
				return "Prefab";
			case AssetType::Terrain:
				return "Terrain";
		}

		throw std::runtime_error("Unknown AssetType");
	}

	void Setup(const std::filesystem::path& path) {
		database = CreateOrLoad(path.string().c_str());
	}

	void Shutdown() {
		Close();
	}

	int8_t Create(void* database) {
		// Create tables
		// Asset table consists of this:
		// - Asset UUID
		// - Asset Name
		// - Asset Type
		// - Asset Blob Path
		if (sqlite3_exec(
			static_cast<sqlite3*>(database),
			"CREATE TABLE IF NOT EXISTS assets (uuid TEXT PRIMARY KEY, name TEXT, updated_at INT8, type TEXT, path TEXT)", nullptr, nullptr, nullptr) != 0) {
			return -1;
		}

		// Asset Package table consists of this:
		// - Package UUID
		// - Package Name
		// - Package Path
		if (sqlite3_exec(
			static_cast<sqlite3*>(database),
			"CREATE TABLE IF NOT EXISTS asset_packages (uuid TEXT PRIMARY KEY, name TEXT, path TEXT)", nullptr, nullptr, nullptr) != 0) {
			return -2;
		}

		// Dependency table consists of this:
		// - Dependency UUID
		// - Asset UUID
		// - Dependency UUID
		if (sqlite3_exec(
			static_cast<sqlite3*>(database),
			"CREATE TABLE IF NOT EXISTS dependencies (uuid TEXT PRIMARY KEY, assetUuid TEXT, dependencyUuid TEXT, FOREIGN KEY(assetUuid) REFERENCES assets(uuid), FOREIGN KEY(dependencyUuid) REFERENCES assets(uuid))", nullptr, nullptr, nullptr) != 0) {
			return -3;
		}

		// Labels table consists of this:
		// - Label UUID
		// - Label name
		// - Label color
		if (sqlite3_exec(
			static_cast<sqlite3*>(database),
			"CREATE TABLE IF NOT EXISTS labels (uuid TEXT PRIMARY KEY, name TEXT, color TEXT)", nullptr, nullptr, nullptr) != 0) {
			return -4;
		}

		// Asset Labels table consists of this:
		// - Asset label UUID
		// - Asset UUID
		// - Label UUID
		if (sqlite3_exec(
			static_cast<sqlite3*>(database),
			"CREATE TABLE IF NOT EXISTS asset_labels (uuid TEXT PRIMARY KEY, assetUuid TEXT, labelUuid TEXT, FOREIGN KEY(assetUuid) REFERENCES assets(uuid), FOREIGN KEY(labelUuid) REFERENCES labels(uuid))", nullptr, nullptr, nullptr) != 0) {
			return -5;
		}

		return 0;
	}

	int AddAsset(
			std::string_view uuid,
			std::string_view name,
			const std::filesystem::path& path,
			const AssetType type,
			const int64_t time
	) {
		sqlite3_stmt* statement;
		sqlite3_prepare_v3(
			static_cast<sqlite3*>(database),
			"INSERT INTO assets (uuid, name, path, type, updated_at) VALUES (?, ?, ?, ?, ?)",
			-1,
			SQLITE_PREPARE_PERSISTENT,
			&statement,
			nullptr
		);

		const auto typeStr = StringFromType(type);

		sqlite3_bind_text(statement, 1, uuid.data(), -1, SQLITE_STATIC);
		sqlite3_bind_text(statement, 2, name.data(), -1, SQLITE_STATIC);
		sqlite3_bind_text(statement, 3, path.string().data(), static_cast<int>(path.string().length()), SQLITE_TRANSIENT);
		sqlite3_bind_text(statement, 4, typeStr.data(), static_cast<int>(typeStr.length()), SQLITE_TRANSIENT);
		sqlite3_bind_int64(statement, 5, time);

		sqlite3_step(statement);
		sqlite3_finalize(statement);

		return sqlite3_errcode(static_cast<sqlite3*>(database));
	}

	void UpdateAsset(std::string_view uuid, int64_t time) {
		// Update the asset's modified time
		sqlite3_stmt* statement;

		sqlite3_prepare_v3(
			static_cast<sqlite3*>(database),
			"UPDATE assets SET updated_at = ? WHERE uuid = ?",
			-1,
			SQLITE_PREPARE_PERSISTENT,
			&statement,
			nullptr
		);

		sqlite3_bind_int64(statement, 1, time);
		sqlite3_bind_text(statement, 2, uuid.data(), -1, SQLITE_STATIC);

		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	void AssetDatabase_RemoveAsset(std::string_view uuid) {
		sqlite3_stmt* statement;
		sqlite3_prepare_v3(
			reinterpret_cast<sqlite3*>(database),
			"DELETE FROM assets WHERE uuid = ? OR path = ?",
			-1,
			SQLITE_PREPARE_PERSISTENT,
			&statement,
			nullptr
		);
		sqlite3_bind_text(statement, 1, uuid.data(), -1, SQLITE_STATIC);

		sqlite3_step(statement);
		sqlite3_finalize(statement);
	}

	int64_t AssetDatabase_GetModifiedTime(const std::string_view uuid) {
		sqlite3_stmt* statement;
		sqlite3_prepare_v3(
			static_cast<sqlite3*>(database),
			"SELECT updated_at FROM assets WHERE uuid = ? LIMIT 1",
			-1,
			SQLITE_PREPARE_PERSISTENT,
			&statement,
			nullptr
		);

		sqlite3_bind_text(statement, 1, uuid.data(), -1, SQLITE_STATIC);

		sqlite3_step(statement);
		auto time = sqlite3_column_int64(statement, 0);
		sqlite3_finalize(statement);

		return time;
	}

	std::string GetUuidForPath(const std::filesystem::path& path) {
		sqlite3_stmt* statement;
		sqlite3_prepare_v3(
			static_cast<sqlite3*>(database),
			"SELECT uuid FROM assets WHERE path = ? LIMIT 1",
			-1,
			SQLITE_PREPARE_PERSISTENT,
			&statement,
			nullptr
		);

		sqlite3_bind_text(statement, 1, path.string().c_str(), -1, SQLITE_STATIC);

		const char* uuidPtr = nullptr;
		while (sqlite3_step(statement) == SQLITE_ROW) {
			uuidPtr = reinterpret_cast<const char*>(sqlite3_column_text(statement, 0));
			if (uuidPtr != nullptr) {
				break;
			}
		}

		sqlite3_finalize(statement);

		if (uuidPtr != nullptr) {
			return { uuidPtr, 37 };
		}

		throw std::runtime_error("Could not find uuid");
	}

	void* CreateOrLoad(const std::filesystem::path& path) {
		sqlite3* db;

		int rc = sqlite3_open(path.string().c_str(), &db);
		if (rc != SQLITE_OK)
		{
			throw std::runtime_error("Could not open database");
		}

		Create(db);

		rc = sqlite3_errcode(db);
		if(rc != SQLITE_OK) {
			throw std::runtime_error("Failed to open database");
		}

		return db;
	}

	void Close() {
		auto* db = static_cast<sqlite3*>(database);
		sqlite3_close(db);
	}
}
