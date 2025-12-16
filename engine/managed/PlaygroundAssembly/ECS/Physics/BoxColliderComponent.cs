using System.Numerics;
using PlaygroundAssembly.ECS.Core;
using Vector3 = PlaygroundAssembly.Math.Vector3;

namespace PlaygroundAssembly.ECS.Physics;

[EcsComponent]
public struct BoxColliderComponent(
    bool isTrigger,
    Vector3 dimensions,
    Vector3 offset,
    Quaternion rotation,
    IntPtr material)
{
    public Vector3 Dimensions = dimensions;
    public Vector3 Offset = offset;
    public Quaternion Rotation = rotation;
    public IntPtr Material = material;

    // Internal runtime handles
    internal ulong Handle = ulong.MaxValue;
    internal ulong BodyHandle = ulong.MaxValue;

    public bool IsTrigger = isTrigger;
}
