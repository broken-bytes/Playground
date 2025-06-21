public struct SunComponent {
    public var direction: Vector3
    public var colour: Colour
    public var intensity: Float

    public init(
        direction: Vector3,
        colour: Colour,
        intensity: Float
    ) {
        self.direction = direction
        self.colour = colour
        self.intensity = intensity
    }
}
