
public enum Renderer {
    internal typealias BatchDrawCalls = @convention(c) (UnsafeRawPointer, UInt16) -> Void
    internal typealias SetSun = @convention(c) (UnsafeRawPointer, UnsafeRawPointer, Float) -> Void
    internal typealias AddCamera = @convention(c) (UInt8, Float, Float, Float, UnsafeRawPointer, UnsafeRawPointer) -> Void

    private static nonisolated(unsafe) var batchDrawCalls: BatchDrawCalls!
    private static nonisolated(unsafe) var setSun: SetSun!
    private static nonisolated(unsafe) var addCameraPtr: AddCamera!

    internal static func setup() {
        batchDrawCalls = NativeLookupTable.getFunctionPointer(by: "Batcher_Batch")
        setSun = NativeLookupTable.getFunctionPointer(by: "Batcher_SetSun")
        addCameraPtr = NativeLookupTable.getFunctionPointer(by: "Batcher_AddCamera")
    }

    internal static nonisolated func batch(_ ptr: UnsafeMutablePointer<DrawCall>, count: UInt16) {
        batchDrawCalls(UnsafeMutableRawPointer(ptr), count)
    }

    internal static nonisolated func setSun(direction: inout Vector3, colour: inout Colour, intensity: Float) {
        setSun(&direction, &colour, intensity)
    }

    internal static func addCamera(order: UInt8, fov: Float, nearPlane: Float, farPlane: Float, position: inout Vector3, rotation: inout Quaternion) {
        addCameraPtr(order, fov, nearPlane, farPlane, &position, &rotation)
    }
}
