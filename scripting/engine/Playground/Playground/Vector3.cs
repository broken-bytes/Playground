namespace Playground;

public struct Vector3(float x, float y, float z)
{
    public float X = x;
    public float Y = y;
    public float Z = z;

    public static Vector3 Zero => new(0f, 0f, 0f);
}
