namespace PlaygroundAssembly.ECS;

public sealed class Mesh
{
    internal IntPtr NativePtr { get; private set; }

    public Mesh()
    {
        NativePtr = IntPtr.Zero;
    }
    
    internal Mesh(IntPtr nativePtr)
    {
        NativePtr = nativePtr;
    }
}
