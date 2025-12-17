namespace Playground.Core.Ecs;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = true, Inherited = false)]
public sealed class EcsQueryItemAttribute(
    Type componentType,
    EcsOperation operation,
    EcsUsage usage)
    : Attribute
{
    public Type ComponentType { get; } = componentType 
                                         ?? throw new ArgumentNullException(nameof(componentType));

    public EcsOperation Operation { get; } = operation;
    public EcsUsage Usage { get; } = usage;
}

