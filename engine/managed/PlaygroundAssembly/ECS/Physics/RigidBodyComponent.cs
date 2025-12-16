using PlaygroundAssembly.ECS.Core;

namespace PlaygroundAssembly.ECS.Physics;

[EcsComponent]
public struct RigidBodyComponent(float mass, float damping)
{
    internal ulong Handle = ulong.MaxValue;
    internal bool IsDirty = true;

    public float Mass = mass;
    public float Damping = damping;
}
