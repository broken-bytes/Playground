@component
public struct SunComponent: Codable {
    public var direction: Vector3
    public var colour: Rendering.Colour
    public var intensity: Float

    public init(
        direction: Vector3,
        colour: Rendering.Colour,
        intensity: Float
    ) {
        self.direction = direction
        self.colour = colour
        self.intensity = intensity
    }
}
