namespace Playground.Core.Ecs;

public readonly struct EcsQueryItem(
    Type componentType,
    EcsOperation operation = EcsOperation.And,
    EcsUsage usage = EcsUsage.Read)
{
    public Type ComponentType { get; } = componentType 
                                         ?? throw new ArgumentNullException(nameof(componentType));

    public EcsOperation Operation { get; } = operation;
    public EcsUsage Usage { get; } = usage;
}
