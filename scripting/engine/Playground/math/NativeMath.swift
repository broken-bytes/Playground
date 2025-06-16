internal enum NativeMath {
    internal typealias Mat4FromPRS = @convention(c) (UnsafeMutableRawPointer, UnsafeMutableRawPointer, UnsafeMutableRawPointer, UnsafeMutableRawPointer) -> Void

    internal static nonisolated(unsafe) var mat4FromPRS: Mat4FromPRS!

    internal static nonisolated func setup() {
        mat4FromPRS = NativeLookupTable.getFunctionPointer(by: "Math_Mat4FromPRS")
    }

    internal static nonisolated func mat4FromPRS(
        _ position: inout Vector3, 
        _ rotation: inout Quaternion, 
        _ scale: inout Vector3,
    ) -> Mat4 {
        var mat4 = Mat4()

        mat4FromPRS(&position, &rotation, &scale, &mat4)

        return mat4
    }
}
