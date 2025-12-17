namespace PlaygroundAssembly.Math;

public interface IVector4<T> where T : unmanaged, IComparable
{
    T X { get; set; }
    T Y { get; set; }
    T Z { get; set; }
    T W { get; set; }
}

public struct Vector4: IVector4<float>
{
    public float X { get; set; }
    public float Y { get; set; }
    public float Z { get; set; }
    public float W { get; set; }

    public readonly float Magnitude()
    {
        return MathF.Sqrt(X * X + Y * Y + Z * Z + W * W);
    }

    public readonly Vector4 Normalised()
    {
        float mag = Magnitude();

        if (mag <= float.Epsilon)
        {
            return default;
        }

        var inv = 1.0f / mag;
        return new Vector4
        {
            X = X * inv,
            Y = Y * inv,
            Z = Z * inv,
            W = W * inv
        };
    }
    
    public void Normalise()
    {
        var mag = Magnitude();

        if (mag <= float.Epsilon)
        {
            return;
        }

        var inv = 1.0f / mag;
        X *= inv;
        Y *= inv;
        Z *= inv;
        W *= inv;
    }
    
    public readonly float MagnitudeSquared()
    {
        return X * X + Y * Y + Z * Z + W * W;
    }
    
    public static Vector4 operator +(Vector4 lhs, Vector4 rhs)
    {
        return new Vector4
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y,
            Z = lhs.Z + rhs.Z,
            W = lhs.W + rhs.W
        };
    }
    
    public static Vector4 operator -(Vector4 lhs, Vector4 rhs)
    {
        return new Vector4
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y,
            Z = lhs.Z - rhs.Z,
            W = lhs.W - rhs.W
        };
    }

    public static Vector4 operator *(Vector4 lhs, float scale)
    {
        return new Vector4
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale,
            Z = lhs.Z * scale,
            W = lhs.W * scale
        };
    }
    
    public static Vector4 operator /(Vector4 lhs, float scale)
    {
        return new Vector4
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale,
            Z = lhs.Z / scale,
            W = lhs.W / scale
        };
    }
}

public struct Vector4Int : IVector4<int>
{
    public int X { get; set; }
    public int Y { get; set; }
    public int Z { get; set; }
    public int W { get; set; }
    
    public static implicit operator Vector4(Vector4Int v)
    {
        return new Vector4
        {
            X = v.X,
            Y = v.Y,
            Z = v.Z,
            W = v.W
        };
    }
    
    public static Vector4Int operator +(Vector4Int lhs, Vector4Int rhs)
    {
        return new Vector4Int
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y,
            Z = lhs.Z + rhs.Z,
            W = lhs.W + rhs.W
        };
    }
    
    public static Vector4Int operator -(Vector4Int lhs, Vector4Int rhs)
    {
        return new Vector4Int
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y,
            Z = lhs.Z - rhs.Z,
            W = lhs.W - rhs.W
        };
    }

    public static Vector4Int operator *(Vector4Int lhs, int scale)
    {
        return new Vector4Int
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale,
            Z = lhs.Z * scale,
            W = lhs.W * scale
        };
    }
    
    public static Vector4Int operator /(Vector4Int lhs, int scale)
    {
        return new Vector4Int
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale,
            Z = lhs.Z / scale,
            W = lhs.W / scale
        };
    }
}
