internal enum ComponentSerialisationManager {
    struct Entry {
        var serialise: (any Codable) -> String
        var deserialise: (String) -> any Codable
    }

    internal static nonisolated(unsafe) var entries: [String:Entry] = [:]
    internal static func addComponent<T>(_ name: String, onSerialise: @escaping (T) -> String, onDeserialise: @escaping (String) -> T) {

    }
}
