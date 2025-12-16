namespace PlaygroundAssembly.ECS.Core;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
public sealed class EcsSystemAttribute(EcsSystemPhase phase = EcsSystemPhase.OnUpdate, int order = 0)
    : Attribute
{
    public EcsSystemPhase Phase { get; } = phase;
    public int Order { get; } = order;
}
