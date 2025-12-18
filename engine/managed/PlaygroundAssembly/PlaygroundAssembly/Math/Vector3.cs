namespace PlaygroundAssembly.Math;


public struct Vector3
{
    public float X { get; set; }
    public float Y { get; set; }
    public float Z { get; set; }

    public readonly float Magnitude()
    {
        return MathF.Sqrt(X * X + Y * Y + Z * Z);
    }

    public readonly Vector3 Normalised()
    {
        float mag = Magnitude();

        if (mag <= float.Epsilon)
        {
            return default;
        }

        var inv = 1.0f / mag;
        return new Vector3
        {
            X = X * inv,
            Y = Y * inv,
            Z = Z * inv
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
    }
    
    public readonly float MagnitudeSquared()
    {
        return X * X + Y * Y + Z * Z;
    }
    
    public static Vector3 operator +(Vector3 lhs, Vector3 rhs)
    {
        return new Vector3
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y,
            Z = lhs.Z + rhs.Z
        };
    }
    
    public static Vector3 operator -(Vector3 lhs, Vector3 rhs)
    {
        return new Vector3
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y,
            Z = lhs.Z - rhs.Z
        };
    }

    public static Vector3 operator *(Vector3 lhs, float scale)
    {
        return new Vector3
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale,
            Z = lhs.Z * scale
        };
    }
    
    public static Vector3 operator /(Vector3 lhs, float scale)
    {
        return new Vector3
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale,
            Z = lhs.Z / scale
        };
    }
}

public struct Vector3Int
{
    public int X { get; set; }
    public int Y { get; set; }
    public int Z { get; set; }
    
    public static implicit operator Vector3(Vector3Int v)
    {
        return new Vector3
        {
            X = v.X,
            Y = v.Y,
            Z = v.Z
        };
    }
    
    public static Vector3Int operator +(Vector3Int lhs, Vector3Int rhs)
    {
        return new Vector3Int
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y,
            Z = lhs.Z + rhs.Z
        };
    }
    
    public static Vector3Int operator -(Vector3Int lhs, Vector3Int rhs)
    {
        return new Vector3Int
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y,
            Z = lhs.Z - rhs.Z
        };
    }

    public static Vector3Int operator *(Vector3Int lhs, int scale)
    {
        return new Vector3Int
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale,
            Z = lhs.Z * scale
        };
    }
    
    public static Vector3Int operator /(Vector3Int lhs, int scale)
    {
        return new Vector3Int
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale,
            Z = lhs.Z / scale
        };
    }
}

