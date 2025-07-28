extension Rendering {
    public struct Colour: Codable, Equatable, Sendable {
        public let r: Float
        public let g: Float
        public let b: Float
        public let a: Float

        public init(r: Float, g: Float, b: Float, a: Float) {
            self.r = r
            self.g = g
            self.b = b
            self.a = a
        }

        public static func ==(lhs: Colour, rhs: Colour) -> Bool {
            lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a
        }
    }
}
