namespace PlaygroundAssembly.Rendering;

public sealed class Material : IDisposable
{
    internal IntPtr  NativePtr { get; private set; }
    private bool _disposed;

    internal Material(IntPtr nativePtr)
    {
        NativePtr = nativePtr;
    }
    
    ~Material() => Dispose();

    public void Dispose()
    {
        if (_disposed)
        {
            return;
        }

        _disposed = true;

        
        if (NativePtr != IntPtr.Zero)
        {
            //NativeRendering.DestroyMaterial(NativePtr);
            NativePtr = IntPtr.Zero;
        }

        GC.SuppressFinalize(this);
    }
}
