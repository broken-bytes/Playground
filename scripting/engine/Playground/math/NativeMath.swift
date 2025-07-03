internal enum NativeMath {
    internal typealias Mat4FromPRS = @convention(c) (UnsafeMutableRawPointer, UnsafeMutableRawPointer, UnsafeMutableRawPointer, UnsafeMutableRawPointer) -> Void
    internal typealias Mat4FromPRSBulk = @convention(c) (UnsafeMutableRawPointer, UnsafeMutableRawPointer, UnsafeMutableRawPointer, UInt64, UnsafeMutableRawPointer) -> Void

    internal static nonisolated(unsafe) var mat4FromPRSPtr: Mat4FromPRS!
    internal static nonisolated(unsafe) var mat4FromPRSBulkPtr: Mat4FromPRSBulk!

    internal static nonisolated func setup() {
        mat4FromPRSPtr = NativeLookupTable.getFunctionPointer(by: "Math_Mat4FromPRS")
        mat4FromPRSBulkPtr = NativeLookupTable.getFunctionPointer(by: "Math_Mat4FromPRSBulk")
    }

    internal static nonisolated func mat4FromPRS(
        _ position: inout Vector3, 
        _ rotation: inout Quaternion, 
        _ scale: inout Vector3,
    ) -> Mat4 {
        var mat4 = Mat4()

        mat4FromPRSPtr(&position, &rotation, &scale, &mat4)

        return mat4
    }

    internal static nonisolated func mat4FromPRSBulk(
        _ position: UnsafeMutablePointer<WorldTranslationComponent>,
        _ rotation: UnsafeMutablePointer<WorldRotationComponent>,
        _ scale: UnsafeMutablePointer<WorldScaleComponent>,
        _ buffer: UnsafeMutableBufferPointer<Mat4>
    ) {
        mat4FromPRSBulkPtr(position, rotation, scale, UInt64(buffer.count), buffer.baseAddress!)
    }
}
