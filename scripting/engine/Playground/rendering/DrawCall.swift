internal extension Rendering {
    internal struct DrawCall {
        var modelHandle: UnsafeMutableRawPointer
        var meshId: UInt16
        var materialHandle: UnsafeMutableRawPointer
        var transform: Mat4
    }
}
