namespace Playground;

public struct Vector4(float x, float y, float z, float w)
{
    public float X = x;
    public float Y = y;
    public float Z = z;
    public float W = w;

    public static Vector4 Zero => new(0f, 0f, 0f, 0f);
    public static Vector4 One => new(1f, 1f, 1f, 1f);
    public static Vector4 Up => new(0f, 1f, 0f, 0f);
    public static Vector4 Down => new(0f, -1f, 0f, 0f);
    public static Vector4 Left => new(-1f, 0f, 0f, 0f);
    public static Vector4 Right => new(1f, 0f, 0f, 0f);
    
    public Vector4(Vector2 vec2) : this(vec2.X, vec2.Y, 0.0f, 0.0f) {}
    public Vector4(Vector3 vec3) : this(vec3.X, vec3.Y, vec3.Z, 0.0f) {}
    public Vector4(Vector4 vec4) : this(vec4.X, vec4.Y, vec4.Z, vec4.W) {}
}
