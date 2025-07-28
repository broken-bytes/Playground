public struct Quaternion: Sendable, Codable {
    public var x: Float
    public var y: Float
    public var z: Float
    public var w: Float

    public var euler: Vector3 {
        toEuler()
    }

    public static let identity: Quaternion = Quaternion(x: 0, y: 0, z: 0, w: 1)

    public init(x: Float, y: Float, z: Float, w: Float) {
        self.x = x
        self.y = y
        self.z = z
        self.w = w
    }

    public init(angle: Float, axis: Vector3) {
        let halfAngle = angle * 0.5
        let sinHalf = sin(halfAngle)
        let cosHalf = cos(halfAngle)
        let normalizedAxis = axis.normalized

        self.init(
            x: normalizedAxis.x * sinHalf,
            y: normalizedAxis.y * sinHalf,
            z: normalizedAxis.z * sinHalf,
            w: cosHalf
        )
    }

    public init(euler angles: Vector3) {
        let cy = cos(angles.z * 0.5)
        let sy = sin(angles.z * 0.5)
        let cp = cos(angles.y * 0.5)
        let sp = sin(angles.y * 0.5)
        let cr = cos(angles.x * 0.5)
        let sr = sin(angles.x * 0.5)

        self.init(
            x: sr * cp * cy - cr * sp * sy,
            y: cr * sp * cy + sr * cp * sy,
            z: cr * cp * sy - sr * sp * cy,
            w: cr * cp * cy + sr * sp * sy
        )
    }

    public static func *(lhs: Quaternion, rhs: Quaternion) -> Quaternion {
        let w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
        let x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y
        let y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x
        let z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w

        return Quaternion(x: x, y: y, z: z, w: w)
    }

    public static func *=(lhs: inout Quaternion, rhs: Quaternion) {
        lhs = lhs * rhs
    }

    public static func *(q: Quaternion, v: Vector3) -> Vector3 {
        let u = Vector3(x: q.x, y: q.y, z: q.z)
        let s = q.w

        let term1 = u * 2.0 * (v .. u)
        let term2 = (s * s - u .. u) * v
        let term3 = 2.0 * s * u ^ v

        return term1 + term2 + term3
    }
}

// - MARK: Helpers
extension Quaternion {
    @inlinable
    public func toEuler() -> Vector3 {
        let ysqr = y * y

        // Roll (X-axis rotation)
        let t0 = +2.0 * (w * x + y * z)
        let t1 = +1.0 - 2.0 * (x * x + ysqr)
        let roll = atan2(t0, t1)

        // Pitch (Y-axis rotation)
        var t2 = +2.0 * (w * y - z * x)
        t2 = t2 > 1.0 ? 1.0 : t2
        t2 = t2 < -1.0 ? -1.0 : t2
        let pitch = asin(t2)

        // Yaw (Z-axis rotation)
        let t3 = +2.0 * (w * z + x * y)
        let t4 = +1.0 - 2.0 * (ysqr + z * z)
        let yaw = atan2(t3, t4)

        return Vector3(x: roll, y: pitch, z: yaw)
    }

    @inlinable
    @inline(__always)
    internal func clamped(_ x: Float, min: Float = -1, max: Float = 1) -> Float {
        Swift.max(min, Swift.min(max, x))
    }

    @inlinable
    @inline(__always)
    public func rotatedAroundLocal(axis: Vector3, angle degrees: Float) -> Quaternion {
        let angle = degToRad(degrees)
        let localAxis = self * axis
        let delta = Quaternion(angle: angle, axis: localAxis)
        return (delta * self).normalized()
    }

    public func normalized() -> Quaternion {
        let len = sqrt(x*x + y*y + z*z + w*w)
        return Quaternion(x: x/len, y: y/len, z: z/len, w: w/len)
    }
}
