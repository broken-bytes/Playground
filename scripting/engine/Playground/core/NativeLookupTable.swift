internal enum NativeLookupTable {
    internal nonisolated(unsafe) static var table: [String: UnsafeRawPointer] = [:]

    internal nonisolated static func getFunctionPointer<T>(by name: String) -> T {
        let ptr = unsafeBitCast(Self.table[name], to: T.self)

        return ptr
    }

    internal nonisolated static func addEntry(key: String, ptr: UnsafeRawPointer) {
        Self.table[key] = ptr
    }
}
