@component
public struct RigidBodyComponent {
    internal var handle: UInt64 = UInt64.max
    public var mass: Float
    public var damping: Float
    internal var isDirty: Bool = true

    public init(mass: Float, damping: Float) {
        self.mass = mass
        self.damping = damping
    }
}
