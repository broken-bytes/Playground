public struct Vector4: Comparable, Codable, Equatable, Sendable {
    public let w: Float
    public let x: Float
    public let y: Float
    public let z: Float

    public var magnitude: Float {
        Math.sqrt(w * w + x * x + y * y + z * z)
    }

    public static let zero = Vector4(w: 0, x: 0, y: 0, z: 0)
    public static let one = Vector4(w: 1, x: 1, y: 1, z: 1)

    public init(w: Float, x: Float, y: Float, z: Float) {
        self.w = w
        self.x = x
        self.y = y
        self.z = z
    }

    public static func < (lhs: Vector4, rhs: Vector4) -> Bool {
        lhs.magnitude < rhs.magnitude
    }

    public static func > (lhs: Vector4, rhs: Vector4) -> Bool {
        lhs.magnitude > rhs.magnitude
    }

    public static func ==(lhs: Vector4, rhs: Vector4) -> Bool {
        lhs.w == rhs.w && lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z
    }
}
