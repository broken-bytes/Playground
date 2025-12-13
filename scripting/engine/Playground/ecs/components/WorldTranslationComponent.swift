@component
public struct WorldTranslationComponent: Codable {
    public var position: Vector3

    public init(
        position: Vector3
    ) {
        self.position = position
    }
}
