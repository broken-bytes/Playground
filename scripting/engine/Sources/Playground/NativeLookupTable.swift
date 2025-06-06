internal actor NativeLookupTable {
    internal static let shared = NativeLookupTable()
    internal nonisolated(unsafe) var table: [String: UnsafeRawPointer] = [:]

    internal nonisolated func getFunctionPointer<T>(by name: String) -> T {
        let ptr = unsafeBitCast(table[name], to: T.self)

        return ptr
    }

    internal nonisolated func addEntry(key: String, ptr: UnsafeRawPointer) {
        table[key] = ptr
    }
}
