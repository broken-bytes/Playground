namespace PlaygroundAssembly.Rendering;

public struct Colour
{
    public static Colour White => new() { R = 1, G = 1, B = 1, A = 1 };
    public static Colour Black => new() { R = 0, G = 0, B = 0, A = 1 };
    public static Colour Transparent => new() { R = 0, G = 0, B = 0, A = 0 };
    public static Colour Red => new() { R = 1, G = 0, B = 0, A = 1 };
    public static Colour Green => new() { R = 0, G = 1, B = 0, A = 1 };
    public static Colour Blue => new() { R = 0, G = 0, B = 1, A = 1 };
    public static Colour Yellow => new() { R = 1, G = 1, B = 0, A = 1 };
    public static Colour Cyan => new() { R = 0, G = 1, B = 1, A = 1 };
    public static Colour Magenta => new() { R = 1, G = 0, B = 1, A = 1 };
    public static Colour Orange => new() { R = 1, G = 0.5f, B = 0, A = 1 };
    public static Colour Purple => new() { R = 0.5f, G = 0, B = 0.5f, A = 1 };
    public static Colour Pink => new() { R = 1, G = 0.75f, B = 0.75f, A = 1 };
    public static Colour Brown => new() { R = 0.6f, G = 0.4f, B = 0.2f, A = 1 };
    public static Colour Gray => new() { R = 0.5f, G = 0.5f, B = 0.5f, A = 1 };

    public float R;
    public float G;
    public float B;
    public float A;
}
