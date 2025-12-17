using Playground.Core.Ecs;
using PlaygroundAssembly.ECS.Core;
using PlaygroundAssembly.Math;

namespace PlaygroundAssembly.ECS.Audio;

[EcsComponent]
public struct AudioSourceComponent
{
    public Vector3 PreviousPosition;
    public Vector3 Forward;
    public ulong ClipId;
    internal IntPtr Handle;
}
