using Playground.Core.Ecs;
using PlaygroundAssembly.Math;

namespace PlaygroundAssembly.ECS.Core;

[EcsComponent]
public struct WorldTransformComponent
{
    public Vector3 Position;
    public Quaternion Rotation;
    public Vector3 Scale;
}
