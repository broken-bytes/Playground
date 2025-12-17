using Playground.Core.Ecs;
using PlaygroundAssembly.ECS.Core;
using PlaygroundAssembly.Rendering;

namespace PlaygroundAssembly.ECS.Rendering;

[EcsComponent]
public struct MaterialComponent
{
    internal IntPtr MaterialPtr;

    public MaterialComponent(Material mesh)
    {
        MaterialPtr = mesh.NativePtr;
    }
    
    internal MaterialComponent(IntPtr materialPtr)
    {
        MaterialPtr = materialPtr;
    }
}
