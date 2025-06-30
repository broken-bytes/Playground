public struct Vector2: Comparable, Codable, Equatable, Sendable {
    public let x: Float
    public let y: Float

    public var magnitude: Float {
        sqrt(x * x + y * y)
    }

    public static let zero = Vector2(x: 0, y: 0)
    public static let one = Vector2(x: 1, y: 1)
    public static let left = Vector2(x: -1, y: 0)
    public static let right = Vector2(x: 1, y: 0)
    public static let up = Vector2(x: 0, y: 1)
    public static let down = Vector2(x: 0, y: -1)

    public init(x: Float, y: Float) {
        self.x = x
        self.y = y
    }

    public static func < (lhs: Vector2, rhs: Vector2) -> Bool {
        lhs.magnitude < rhs.magnitude
    }

    public static func > (lhs: Vector2, rhs: Vector2) -> Bool {
        lhs.magnitude > rhs.magnitude
    }

    public static func ==(lhs: Vector2, rhs: Vector2) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y
    }
}
