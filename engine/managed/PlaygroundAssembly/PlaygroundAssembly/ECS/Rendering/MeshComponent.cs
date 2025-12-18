using System.Runtime.InteropServices;
using Playground.Core.Ecs;
using PlaygroundAssembly.ECS.Core;

namespace PlaygroundAssembly.ECS.Rendering;

[EcsComponent]
[StructLayout(LayoutKind.Explicit, Pack = 8, Size = 8)]
public struct MeshComponent
{
    [FieldOffset(0)]
    public ulong AssetId;
}
