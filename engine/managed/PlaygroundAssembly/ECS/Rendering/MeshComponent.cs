using System.Runtime.InteropServices;
using PlaygroundAssembly.ECS.Core;

namespace PlaygroundAssembly.ECS.Rendering;

[EcsComponent]
[StructLayout(LayoutKind.Explicit, Size = 16)]
public struct MeshComponent
{
    [FieldOffset(0)]
    internal IntPtr ModelPtr;
    [FieldOffset(8)]
    internal ushort MeshId;

    public MeshComponent(Mesh mesh)
    {
        ModelPtr = mesh.NativePtr;
        MeshId = 0;
    }
    
    internal MeshComponent(IntPtr meshPtr)
    {
        ModelPtr = meshPtr;
    }
}
