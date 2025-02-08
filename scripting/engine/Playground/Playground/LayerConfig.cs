namespace Playground;

public static class LayerConfig
{
    static Dictionary<LayerMask, string> _layers = new();
    
    static LayerConfig()
    {
        
    }

    internal static void SetLayer(LayerMask layerMask, string layerName)
    {
        _layers[layerMask] = layerName;
    }
}
