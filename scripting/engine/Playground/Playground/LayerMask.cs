namespace Playground;

public class LayerMask
{
    private int _mask;

    public LayerMask() : this(0) {}

    public LayerMask(int mask)
    {
        _mask = mask;
    }
    
    public static int GetMask(string layers)
    {
        return 0;
    }

    public static string LayerName(int layer)
    {
        return "Layer" + layer;
    }
    
    public static implicit operator LayerMask(int mask) => new LayerMask(mask);
}
