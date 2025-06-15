
public enum Renderer {
    internal typealias BatchDrawCalls = @convention(c) (UnsafeRawPointer, UInt16) -> Void

    private static nonisolated(unsafe) var batchDrawCalls: BatchDrawCalls!

    internal static func setup() {
        batchDrawCalls = NativeLookupTable.getFunctionPointer(by: "Batcher_Batch")
    }

    internal static nonisolated func batch(_ ptr: UnsafeMutablePointer<DrawCall>, count: UInt16) {
        batchDrawCalls(UnsafeMutableRawPointer(ptr), count)
    }
}
