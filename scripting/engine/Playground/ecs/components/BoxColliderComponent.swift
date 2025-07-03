public struct BoxColliderComponent {
    internal var isDirty: Bool = true

    public var dimensions: Vector3 = .zero {
        didSet {
            isDirty = true
        }
    }

    public var offset: Vector3 = .zero {
        didSet {
            isDirty = true
        }
    }

    public var rotation: Quaternion = .identity {
        didSet {
            isDirty = true
        }
    }

    public var material: UnsafeMutableRawPointer {
        didSet {
            isDirty = true
        }
    }

    internal var handle: UInt64! = nil

    public init(dimensions: Vector3, offset: Vector3, rotation: Quaternion, material: UnsafeMutableRawPointer) {
        self.dimensions = dimensions
        self.offset = offset
        self.rotation = rotation
        self.material = material
    }
}
