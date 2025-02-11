namespace Playground;

public struct Vector3(float x, float y, float z)
{
    public float X = x;
    public float Y = y;
    public float Z = z;

    public static Vector3 Zero => new(0f, 0f, 0f);
    public static Vector3 One => new(1f, 1f, 1f);
    public static Vector3 Up => new(0f, 1f, 0f);
    public static Vector3 Down => new(0f, -1f, 0f);
    public static Vector3 Left => new(-1f, 0f, 0f);
    public static Vector3 Right => new(1f, 0f, 0f);
    
    public Vector3(Vector2 vec2) : this(vec2.X, vec2.Y, 0.0f) {}
    public Vector3(Vector3 vec3) : this(vec3.X, vec3.Y, vec3.Z) {}
    public Vector3(Vector4 vec4) : this(vec4.X, vec4.Y, vec4.Z) {}
}
