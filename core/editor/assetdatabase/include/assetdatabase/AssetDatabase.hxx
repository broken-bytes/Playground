#pragma once

#include <cstdint>
#include <filesystem>
#include <string_view>
#include "assetdatabase/AssetType.hxx"

namespace playground::editor::assetdatabase
{
	/**
	 * Setups the editor database
	 * @param path The path to be used for the database
	 */
	void Setup(const std::filesystem::path& path);

	/**
	 * Shuts the database down
	 */
	void Shutdown();

	/**
	* @brief Adds an asset to the database.
	* @param uuid The UUID of the asset.
	* @param name The name of the asset.
	* @param path The path of the asset.
	* @param type The type of the asset.
	* @param time The time the asset was last modified.
	* @return True if the asset was added, false if the asset already exists.
	*/
	int AddAsset(
		std::string_view uuid,
		std::string_view name,
		const std::filesystem::path& path,
		AssetType type,
		int64_t time
	);

	/**
	* @brief Updates the asset in the database.
	* @param uuid The UUID of the asset.
	* @param time The time the asset was last modified.
	*/
	void UpdateAsset(std::string_view uuid, int64_t time);

	/**
	* @brief Removes an asset from the database.
	* @param uuid The UUID of the asset.
	*/
	void RemoveAsset(std::string_view uuid);

	/**
	* @brief Gets the last modified time of an asset.
	* @param uuid The UUID of the asset.
	* @return The last modified time of the asset.
	*/
	int64_t GetModifiedTime(std::string_view uuid);

	/**
	* @brief Gets the UUID of an asset by its path.
	* @param path The path of the asset.
	* @param uuid The UUID of the asset. If the asset does not exist, this will be set to NULL.
	*/
	std::string GetUuidForPath(const std::filesystem::path& path, std::string_view uuid);
}
