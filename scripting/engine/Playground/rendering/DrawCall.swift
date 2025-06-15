internal struct DrawCall {
    var modelHandle: UnsafeMutableRawPointer
    var meshId: UInt16
    var materialHandle: UnsafeMutableRawPointer
    var position: Vector3
    var rotation: Quaternion
    var scale: Vector3
}
