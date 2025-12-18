namespace PlaygroundAssembly.Rendering;

public sealed class Material
{
    internal IntPtr NativePtr { get; private set; }

    public Material()
    {
        NativePtr = IntPtr.Zero;
    }

    ~Material()
    {
        // TODO: Destroy material in native
        NativePtr = IntPtr.Zero;
    }
    
    internal Material(IntPtr nativePtr)
    {
        NativePtr = nativePtr;
    }
}
