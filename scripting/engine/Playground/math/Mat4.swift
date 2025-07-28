public struct Mat4: Codable {
    public var m00: Float, m10: Float, m20: Float, m30: Float
    public var m01: Float, m11: Float, m21: Float, m31: Float
    public var m02: Float, m12: Float, m22: Float, m32: Float
    public var m03: Float, m13: Float, m23: Float, m33: Float

    public init() {
        self = Mat4.identity()
    }

    public static func identity() -> Mat4 {
        return Mat4(
            m00: 1, m10: 0, m20: 0, m30: 0,
            m01: 0, m11: 1, m21: 0, m31: 0,
            m02: 0, m12: 0, m22: 1, m32: 0,
            m03: 0, m13: 0, m23: 0, m33: 1
        )
    }

    public init(
        m00: Float, m10: Float, m20: Float, m30: Float,
        m01: Float, m11: Float, m21: Float, m31: Float,
        m02: Float, m12: Float, m22: Float, m32: Float,
        m03: Float, m13: Float, m23: Float, m33: Float
    ) {
        self.m00 = m00; self.m10 = m10; self.m20 = m20; self.m30 = m30
        self.m01 = m01; self.m11 = m11; self.m21 = m21; self.m31 = m31
        self.m02 = m02; self.m12 = m12; self.m22 = m22; self.m32 = m32
        self.m03 = m03; self.m13 = m13; self.m23 = m23; self.m33 = m33
    }
}
