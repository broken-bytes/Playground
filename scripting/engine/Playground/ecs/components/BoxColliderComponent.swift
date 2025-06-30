public struct BoxColliderComponent {
    internal var isDirty: Bool = true

    public var dimensions: Vector3 = 0 {
        didSet {
            isDirty = true
        }
    }

    public var offset: Vector3 = 0 {
        didSet {
            isDirty = true
        }
    }

    public var material: UnsafeMutableRawPointer {
        didSet {
            isDirty = true
        }
    }

    public init(dimensions: Vector3, offset: Vector3, material: UnsafeMutableRawPointer) {
        self.dimensions = dimensions
        self.offset = offset
        self.material = material
    }
}
