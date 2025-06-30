public struct TranslationComponent {
    internal var isDirty: Bool = true

    public var position: Vector3 {
        didSet {
            isDirty = true
        }
    }

    public init(
        position: Vector3
    ) {
        self.position = position
    }
}
