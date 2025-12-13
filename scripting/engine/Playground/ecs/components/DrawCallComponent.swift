@component
@component
internal struct DrawCallComponent {
    let modelHandle: UnsafeMutableRawPointer
    let materialHandle: UnsafeMutableRawPointer
    let meshId: UInt16
    let position: Vector3
    let rotation: Quaternion
    let scale: Vector3
}
