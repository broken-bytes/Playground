public struct RigidbodyComponent {
    internal var isDirty: Bool = true
    internal var handle: UInt64? = nil

    public var mass: Float {
        didSet {
            isDirty = true
        }
    }

    public var damping: Float {
        didSet {
            isDirty = true
        }
    }

    public init(mass: Float, damping: Float) {
        self.mass = mass
        self.damping = damping
    }
}
