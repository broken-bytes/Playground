public struct WorldScaleComponent: Codable {
    public var scale: Vector3

    public init(scale: Vector3) {
        self.scale = scale
    }
}
