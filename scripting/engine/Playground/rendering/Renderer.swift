
public enum Renderer {
    internal typealias BatchDrawCalls = @convention(c) (UnsafeRawPointer, UInt16) -> Void
    internal typealias SetSun = @convention(c) (UnsafeRawPointer, UnsafeRawPointer, Float) -> Void

    private static nonisolated(unsafe) var batchDrawCalls: BatchDrawCalls!
    private static nonisolated(unsafe) var setSun: SetSun!

    internal static func setup() {
        batchDrawCalls = NativeLookupTable.getFunctionPointer(by: "Batcher_Batch")
        setSun = NativeLookupTable.getFunctionPointer(by: "Batcher_SetSun")
    }

    internal static nonisolated func batch(_ ptr: UnsafeMutablePointer<DrawCall>, count: UInt16) {
        batchDrawCalls(UnsafeMutableRawPointer(ptr), count)
    }

    internal static nonisolated func setSun(direction: inout Vector3, colour: inout Colour, intensity: Float) {
        setSun(&direction, &colour, intensity)
    }
}
