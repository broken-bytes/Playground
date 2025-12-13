@_alignment(8)
@frozen
@component
public struct BoxColliderComponent {
    public var dimensions: Vector3 = .zero
    public var offset: Vector3 = .zero
    public var rotation: Quaternion = .identity
    public var material: UnsafeMutableRawPointer
    internal var handle: UInt64 = UInt64.max
    internal var bodyHandle: UInt64 = UInt64.max
    public var isTrigger: Bool = false

    public init(isTrigger: Bool, dimensions: Vector3, offset: Vector3, rotation: Quaternion, material: UnsafeMutableRawPointer) {
        self.isTrigger = isTrigger
        self.dimensions = dimensions
        self.offset = offset
        self.rotation = rotation
        self.material = material
    }
}
