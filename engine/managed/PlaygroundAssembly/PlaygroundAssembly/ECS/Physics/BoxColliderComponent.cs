using System.Numerics;
using System.Runtime.InteropServices;
using Playground.Core.Ecs;
using PlaygroundAssembly.ECS.Core;
using Vector3 = PlaygroundAssembly.Math.Vector3;

namespace PlaygroundAssembly.ECS.Physics;

[EcsComponent]
[StructLayout(LayoutKind.Explicit, Pack = 16, Size = 80)]
public struct BoxColliderComponent(
    bool isTrigger,
    Vector3 dimensions,
    Vector3 offset,
    Quaternion rotation,
    IntPtr material)
{
    [FieldOffset(0)]
    public Vector3 Dimensions = dimensions;
    [FieldOffset(12)]
    public Vector3 Offset = offset;
    [FieldOffset(24)]
    public Quaternion Rotation = rotation;
    [FieldOffset(40)]
    public IntPtr Material = material;
    // Internal runtime handles
    [FieldOffset(48)]
    internal ulong Handle = ulong.MaxValue;
    [FieldOffset(56)]
    internal ulong BodyHandle = ulong.MaxValue;
    [FieldOffset(64)]
    public bool IsTrigger = isTrigger;
}
