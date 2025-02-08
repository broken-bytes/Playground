namespace Playground;

public partial class RenderTexture
{
    public uint Width { get; }
    public uint Height { get; }

    public RenderTexture(uint width, uint height)
    {
        Width = width;
        Height = height;
    }
}
