infix operator ..: MultiplicationPrecedence
infix operator ^: MultiplicationPrecedence

public struct Vector3: Comparable, Codable, Equatable, Sendable {
    public let x: Float
    public let y: Float
    public let z: Float

    public var magnitude: Float {
        Math.sqrt(x * x + y * y + z * z)
    }

    public var normalized: Vector3 {
        let len = Math.sqrt(x * x + y * y + z * z)
        guard len != 0 else { return self }

        return self / len
    }

    public static let zero = Vector3(x: 0, y: 0, z: 0)
    public static let one = Vector3(x: 1, y: 1, z: 1)
    public static let left = Vector3(x: -1, y: 0, z: 0)
    public static let right = Vector3(x: 1, y: 0, z: 0)
    public static let up = Vector3(x: 0, y: 1, z: 0)
    public static let down = Vector3(x: 0, y: -1, z: 0)
    public static let forward = Vector3(x: 0, y: 0, z: 1)
    public static let backward = Vector3(x: 0, y: 0, z: -1)

    public init(x: Float, y: Float, z: Float) {
        self.x = x
        self.y = y
        self.z = z
    }

    public func dot(_ other: Vector3) -> Float {
        return x * other.x + y * other.y + z * other.z
    }

    public func cross(_ other: Vector3) -> Vector3 {
        return Vector3(
            x: y * other.z - z * other.y,
            y: z * other.x - x * other.z,
            z: x * other.y - y * other.x
        )
    }

    public static func < (lhs: Vector3, rhs: Vector3) -> Bool {
        lhs.magnitude < rhs.magnitude
    }

    public static func > (lhs: Vector3, rhs: Vector3) -> Bool {
        lhs.magnitude > rhs.magnitude
    }

    public static func ==(lhs: Vector3, rhs: Vector3) -> Bool {
        lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z
    }

    public static func +(lhs: Vector3, rhs: Vector3) -> Vector3 {
        Vector3(x: lhs.x + rhs.x, y: lhs.y + rhs.y, z: lhs.z + rhs.z)
    }

    public static func +=(lhs: inout Vector3, rhs: Vector3) {
        lhs = lhs + rhs
    }

    public static func /(lhs: Vector3, rhs: Float) -> Vector3 {
        Vector3(x: lhs.x / rhs, y: lhs.y / rhs, z: lhs.z / rhs)
    }

    public static func *(lhs: Vector3, rhs: Vector3) -> Vector3 {
        Vector3(
            x: lhs.x * rhs.x,
            y: lhs.y * rhs.y,
            z: lhs.z * rhs.z
        )
    }

    public static func *(v: Vector3, scale: Float) -> Vector3 {
        Vector3(
            x: v.x * scale,
            y: v.y * scale,
            z: v.z * scale
        )
    }

    public static func *(scale: Float, v: Vector3) -> Vector3 {
        Vector3(
            x: v.x * scale,
            y: v.y * scale,
            z: v.z * scale
        )
    }

    public static func *=(v: inout Vector3, scale: Float) {
        v = v * scale
    }

    public static func ..(lhs: Vector3, rhs: Vector3) -> Float {
        lhs.dot(rhs)
    }

    public static func ^(lhs: Vector3, rhs: Vector3) -> Vector3 {
        lhs.cross(rhs)
    }
}
