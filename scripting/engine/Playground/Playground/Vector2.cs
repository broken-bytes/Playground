namespace Playground;

public struct Vector2(float x, float y)
{
    public float X = x;
    public float Y = y;

    public static Vector2 Zero => new(0f, 0f);
    public static Vector2 One => new(1f, 1f);
    public static Vector2 Up => new(0f, 1f);
    public static Vector2 Down => new(0, -1f);
    public static Vector2 Left => new(-1f, 0);
    public static Vector2 Right => new(1f, 0);
    
    public Vector2(Vector2 vec2) : this(vec2.X, vec2.Y) {}
    public Vector2(Vector3 vec3) : this(vec3.X, vec3.Y) {}
    public Vector2(Vector4 vec4) : this(vec4.X, vec4.Y) {}
}
