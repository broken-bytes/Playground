namespace PlaygroundAssembly.Math;

public struct Vector2
{
    public float X { get; set; }
    public float Y { get; set; }

    public readonly float Magnitude()
    {
        return MathF.Sqrt(X * X + Y * Y);
    }

    public readonly Vector2 Normalised()
    {
        float mag = Magnitude();

        if (mag <= float.Epsilon)
        {
            return default;
        }

        var inv = 1.0f / mag;
        return new Vector2
        {
            X = X * inv,
            Y = Y * inv
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
    }
    
    public readonly float MagnitudeSquared()
    {
        return X * X + Y * Y;
    }

    public static Vector2 operator +(Vector2 lhs, Vector2 rhs)
    {
        return new Vector2
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y
        };
    }
    
    public static Vector2 operator -(Vector2 lhs, Vector2 rhs)
    {
        return new Vector2
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y
        };
    }

    public static Vector2 operator *(Vector2 lhs, float scale)
    {
        return new Vector2
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale
        };
    }
    
    public static Vector2 operator /(Vector2 lhs, float scale)
    {
        return new Vector2
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale
        };
    }
}

public struct Vector2Int
{
    public int X { get; set; }
    public int Y { get; set; }
    
    public static implicit operator Vector2(Vector2Int v)
    {
        return new Vector2
        {
            X = v.X,
            Y = v.Y
        };
    }
    
    public static Vector2Int operator +(Vector2Int lhs, Vector2Int rhs)
    {
        return new Vector2Int
        {
            X = lhs.X + rhs.X,
            Y = lhs.Y + rhs.Y
        };
    }
    
    public static Vector2 operator -(Vector2Int lhs, Vector2Int rhs)
    {
        return new Vector2
        {
            X = lhs.X - rhs.X,
            Y = lhs.Y - rhs.Y
        };
    }

    public static Vector2Int operator *(Vector2Int lhs, int scale)
    {
        return new Vector2Int
        {
            X = lhs.X * scale,
            Y = lhs.Y * scale
        };
    }
    
    public static Vector2Int operator /(Vector2Int lhs, int scale)
    {
        return new Vector2Int
        {
            X = lhs.X / scale,
            Y = lhs.Y / scale
        };
    }
}

