using Microsoft.Data.Sqlite;
using System;
using System.IO;

namespace PlaygroundEditor
{
    public static class AssetDatabase
    {
        public enum AssetType
        {
            AudioClip,
            Material,
            Mesh,
            Shader,
            Texture
        }
        private static SqliteConnection _database;

        internal static void Setup(string path)
        {
            _database = CreateOrLoad(path);
        }

        internal static void Shutdown()
        {
            Close();
        }

        internal static SqliteConnection CreateOrLoad(string path)
        {
            var connectionString = $"Data Source={path}";
            var db = new SqliteConnection(connectionString);

            try
            {
                db.Open();
                Create(db);
            }
            catch (Exception)
            {
                throw new Exception("Could not open database");
            }

            return db;
        }

        internal static void Close()
        {
            _database.Close();
        }

        private static void Create(SqliteConnection db)
        {
            using (var transaction = db.BeginTransaction())
            {
                // Create assets table
                var createAssetsTableQuery = @"
                    CREATE TABLE IF NOT EXISTS assets (
                        uuid TEXT PRIMARY KEY,
                        name TEXT,
                        updated_at INTEGER,
                        type TEXT,
                        path TEXT
                    )";
                ExecuteNonQuery(db, createAssetsTableQuery);

                // Create asset_packages table
                var createPackagesTableQuery = @"
                    CREATE TABLE IF NOT EXISTS asset_packages (
                        uuid TEXT PRIMARY KEY,
                        name TEXT,
                        path TEXT
                    )";
                ExecuteNonQuery(db, createPackagesTableQuery);

                // Create dependencies table
                var createDependenciesTableQuery = @"
                    CREATE TABLE IF NOT EXISTS dependencies (
                        uuid TEXT PRIMARY KEY,
                        assetUuid TEXT,
                        dependencyUuid TEXT,
                        FOREIGN KEY(assetUuid) REFERENCES assets(uuid),
                        FOREIGN KEY(dependencyUuid) REFERENCES assets(uuid)
                    )";
                ExecuteNonQuery(db, createDependenciesTableQuery);

                // Create labels table
                var createLabelsTableQuery = @"
                    CREATE TABLE IF NOT EXISTS labels (
                        uuid TEXT PRIMARY KEY,
                        name TEXT,
                        color TEXT
                    )";
                ExecuteNonQuery(db, createLabelsTableQuery);

                // Create asset_labels table
                var createAssetLabelsTableQuery = @"
                    CREATE TABLE IF NOT EXISTS asset_labels (
                        uuid TEXT PRIMARY KEY,
                        assetUuid TEXT,
                        labelUuid TEXT,
                        FOREIGN KEY(assetUuid) REFERENCES assets(uuid),
                        FOREIGN KEY(labelUuid) REFERENCES labels(uuid)
                    )";
                ExecuteNonQuery(db, createAssetLabelsTableQuery);

                transaction.Commit();
            }
        }

        private static void ExecuteNonQuery(SqliteConnection db, string query)
        {
            using (var cmd = new SqliteCommand(query, db))
            {
                cmd.ExecuteNonQuery();
            }
        }

        internal static void AddAsset(string uuid, string name, string path, AssetType type, long time)
        {
            using (var transaction = _database.BeginTransaction())
            {
                var insertQuery = @"
                    INSERT INTO assets (uuid, name, path, type, updated_at)
                    VALUES (@uuid, @name, @path, @type, @updated_at)";
                using (var cmd = new SqliteCommand(insertQuery, _database))
                {
                    cmd.Parameters.AddWithValue("@uuid", uuid);
                    cmd.Parameters.AddWithValue("@name", name);
                    cmd.Parameters.AddWithValue("@path", path);
                    cmd.Parameters.AddWithValue("@type", StringFromType(type));
                    cmd.Parameters.AddWithValue("@updated_at", time);

                    cmd.ExecuteNonQuery();
                }

                transaction.Commit();
            }
        }

        internal static void UpdateAsset(string uuid, long time)
        {
            using (var transaction = _database.BeginTransaction())
            {
                var updateQuery = @"
                    UPDATE assets 
                    SET updated_at = @updated_at 
                    WHERE uuid = @uuid";
                using (var cmd = new SqliteCommand(updateQuery, _database))
                {
                    cmd.Parameters.AddWithValue("@updated_at", time);
                    cmd.Parameters.AddWithValue("@uuid", uuid);

                    cmd.ExecuteNonQuery();
                }

                transaction.Commit();
            }
        }

        internal static void RemoveAsset(string uuid)
        {
            using (var transaction = _database.BeginTransaction())
            {
                var deleteQuery = @"
                    DELETE FROM assets 
                    WHERE uuid = @uuid OR path = @path";
                using (var cmd = new SqliteCommand(deleteQuery, _database))
                {
                    cmd.Parameters.AddWithValue("@uuid", uuid);
                    cmd.Parameters.AddWithValue("@path", uuid);  // Assuming path is also used as identifier

                    cmd.ExecuteNonQuery();
                }

                transaction.Commit();
            }
        }

        internal static long GetModifiedTime(string uuid)
        {
            var selectQuery = "SELECT updated_at FROM assets WHERE uuid = @uuid LIMIT 1";
            using (var cmd = new SqliteCommand(selectQuery, _database))
            {
                cmd.Parameters.AddWithValue("@uuid", uuid);

                using (var reader = cmd.ExecuteReader())
                {
                    if (reader.Read())
                    {
                        return reader.GetInt64(0);
                    }
                }
            }

            throw new Exception("Could not find uuid");
        }

        internal static string GetUuidForPath(string path)
        {
            var selectQuery = "SELECT uuid FROM assets WHERE path = @path LIMIT 1";
            using (var cmd = new SqliteCommand(selectQuery, _database))
            {
                cmd.Parameters.AddWithValue("@path", path);

                using (var reader = cmd.ExecuteReader())
                {
                    if (reader.Read())
                    {
                        return reader.GetString(0);
                    }
                }
            }

            throw new Exception("Could not find uuid");
        }

        internal static string StringFromType(AssetType type)
        {
            switch (type)
            {
                case AssetType.AudioClip: return "Audio";
                case AssetType.Material: return "Material";
                case AssetType.Mesh: return "Model";
                case AssetType.Shader: return "Shader";
                case AssetType.Texture: return "Texture";
              
                default:
                    throw new Exception("Unknown AssetType");
            }
        }
    }
}
