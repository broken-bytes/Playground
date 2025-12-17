namespace Playground.Core.Ecs;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public sealed class EcsSystemAttribute(EcsSystemPhase phase = EcsSystemPhase.OnUpdate, int order = 0, bool isMultiThreaded = false)
    : Attribute
{
    public EcsSystemPhase Phase { get; } = phase;
    public int Order { get; } = order;
    public bool IsMultiThreaded { get; set; } = false;
}
