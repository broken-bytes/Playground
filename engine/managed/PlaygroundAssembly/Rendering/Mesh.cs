namespace PlaygroundAssembly.ECS;

public sealed class Mesh : IDisposable
{
    internal IntPtr NativePtr { get; private set; }
    private bool _disposed;

    internal Mesh(IntPtr nativePtr)
    {
        NativePtr = nativePtr;
    }
    
    ~Mesh() => Dispose();

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
