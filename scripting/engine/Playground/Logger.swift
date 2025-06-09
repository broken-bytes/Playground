public enum Logger {
    internal typealias Log = @convention(c) (UnsafePointer<CChar>) -> Void

    internal static nonisolated(unsafe) var infoPtr: Log!
    internal static nonisolated(unsafe) var warnPtr: Log!
    internal static nonisolated(unsafe) var errorPtr: Log!

    internal static nonisolated func start() {
        infoPtr = NativeLookupTable.shared.getFunctionPointer(by: "Logger_Info")
        warnPtr = NativeLookupTable.shared.getFunctionPointer(by: "Logger_Warn")
        errorPtr = NativeLookupTable.shared.getFunctionPointer(by: "Logger_Error")
    }

    public static func info(_ message: String) {
        message.withCString { infoPtr($0) }
    }

    public static func warn(_ message: String) {
        message.withCString { warnPtr($0) }
    }

    public static func error(_ message: String) {
        message.withCString { errorPtr($0) }
    }
}
