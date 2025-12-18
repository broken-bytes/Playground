using System.Runtime.InteropServices;
using Playground.Core.Ecs;
using PlaygroundAssembly.Math;

namespace PlaygroundAssembly.ECS.Core;

[EcsComponent]
[StructLayout(LayoutKind.Explicit, Pack = 16, Size = 48)]
public struct WorldTransformComponent
{
    [FieldOffset(0)]
    public Vector3 Position;
    [FieldOffset(12)]
    public Quaternion Rotation;
    [FieldOffset(28)]
    public Vector3 Scale;
}
;
