@frozen
@component
public struct CameraComponent: Codable {
    public var order: UInt8
    public var fov: Float
    public var nearPlane: Float
    public var farPlane: Float

    public init(
        order: UInt8,
        fov: Float,
        nearPlane: Float,
        farPlane: Float
    ) {
        self.order = order
        self.fov = fov
        self.nearPlane = nearPlane
        self.farPlane = farPlane
    }
}
