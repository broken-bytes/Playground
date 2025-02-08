namespace Playground;

[Serializable]
public struct Colour(float r, float g, float b, float a)
{
    public float R = r;
    public float G = g;
    public float B = b;
    public float A = a;

    public Colour(byte r, byte g, byte b, byte a) : this(r / 255f, g / 255f, b / 255f, a / 255f) { }
    
    public static Colour White => new() { R = 1, G = 1, B = 1, A = 1 };
    public static Colour Black => new() { R = 0, G = 0, B = 0, A = 1 };
    public static Colour Gray => new() { R = 0.5f, G = 0.5f, B = 0.5f, A = 1 };
    public static Colour Red => new() { R = 1, G = 0, B = 0, A = 1 };
    public static Colour Green => new() { R = 0, G = 1, B = 0, A = 1 };
    public static Colour Blue => new() { R = 0, G = 0, B = 1, A = 1 };
    public static Colour Yellow => new() { R = 1, G = 1, B = 0, A = 1 };
    public static Colour Cyan => new() { R = 0.5f, G = 0.5f, B = 1, A = 1 };
    public static Colour Magenta => new() { R = 1, G = 0.25f, B = 0.25f, A = 1 };
}
