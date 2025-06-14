public enum Math {
    @inlinable
    public static func sqrt(_ x: Float) -> Float {
        var guess = x / 2
        let epsilon: Float = 0.00001

        while Math.abs(guess * guess - x) > epsilon {
            guess = (guess + x / guess) / 2
        }

        return guess
    }

    @inlinable
    public static func sin(_ x: Float) -> Float {
        let x = x.truncatingRemainder(dividingBy: 2 * .pi)
        return x - (x * x * x) / 6 + (x * x * x * x * x) / 120
    }

    @inlinable
    public static func cos(_ x: Float) -> Float {
        let x = x.truncatingRemainder(dividingBy: 2 * .pi)
        return 1 - (x * x) / 2 + (x * x * x * x) / 24
    }

    @inlinable
    public static func asin(_ x: Float) -> Float {
        // Polynomial approximation valid for small |x|
        let x3 = x * x * x
        let x5 = x3 * x * x
        return x + (x3 / 6) + (3 * x5 / 40)
    }

    @inlinable
    public static func atan2(_ y: Float, _ x: Float) -> Float {
        if x == 0 {
            return y > 0 ? .pi / 2 : -(.pi / 2)
        }
        let atan = y / x
        if Math.abs(atan) < 1 {
            return atan
        }
        return (y > 0 ? .pi : -.pi) - atan
    }

    @inlinable
    public static func abs(_ value: Float) -> Float {
        return value < 0 ? -value : value
    }

    @inlinable
    public static func sqrt(_ x: Double) -> Double {
        var guess = x / 2
        let epsilon: Double = 0.0000001

        while Math.abs(guess * guess - x) > epsilon {
            guess = (guess + x / guess) / 2
        }

        return guess
    }

    @inlinable
    public static func sin(_ x: Double) -> Double {
        let x = x.truncatingRemainder(dividingBy: 2 * .pi)
        return x - (x * x * x) / 6 + (x * x * x * x * x) / 120
    }

    @inlinable
    public static func cos(_ x: Double) -> Double {
        let x = x.truncatingRemainder(dividingBy: 2 * .pi)
        return 1 - (x * x) / 2 + (x * x * x * x) / 24
    }

    @inlinable
    public static func asin(_ x: Double) -> Double {
        let x3 = x * x * x
        let x5 = x3 * x * x
        return x + (x3 / 6) + (3 * x5 / 40)
    }

    @inlinable
    public static func atan2(_ y: Double, _ x: Double) -> Double {
        if x == 0 {
            return y > 0 ? .pi / 2 : -(.pi / 2)
        }
        let atan = y / x
        if Math.abs(atan) < 1 {
            return atan
        }
        return (y > 0 ? .pi : -.pi) - atan
    }

    @inlinable
    public static func abs(_ value: Double) -> Double {
        return value < 0 ? -value : value
    }

    @inlinable
    public static func sqrt(_ x: Int) -> Double {
        return Math.sqrt(Double(x))
    }

    @inlinable
    public static func sin(_ x: Int) -> Double {
        return Math.sin(Double(x))
    }

    @inlinable
    public static func cos(_ x: Int) -> Double {
        return Math.cos(Double(x))
    }

    @inlinable
    public static func asin(_ x: Int) -> Double {
        return Math.asin(Double(x))
    }

    @inlinable
    public static func atan2(y: Int, x: Int) -> Double {
        return Math.atan2(Double(y), Double(x))
    }

    @inlinable
    public static func abs(_ value: Int) -> Int {
        return value < 0 ? -value : value
    }
}
