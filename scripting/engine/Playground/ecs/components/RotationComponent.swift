public struct RotationComponent: Codable {
    public var rotation: Quaternion

    public init(rotation: Quaternion) {
        self.rotation = rotation
    }
}
