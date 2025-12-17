using Playground.Core.Ecs;
using PlaygroundAssembly.ECS.Core;
using PlaygroundAssembly.Math;
using PlaygroundAssembly.Rendering;

namespace PlaygroundAssembly.ECS.Rendering;

[EcsComponent]
public struct SunComponent
{
    public Vector3 Direction;
    public Colour Colour;
    public float Intensity;
}
