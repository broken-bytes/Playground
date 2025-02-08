namespace Playground;

public struct Quaternion(float x, float y, float z, float w)
{
    public float X = x;
    public float Y = y;
    public float Z = z;
    public float W = w;

    public static Quaternion Identity => new(0, 0, 0, 1);
}
