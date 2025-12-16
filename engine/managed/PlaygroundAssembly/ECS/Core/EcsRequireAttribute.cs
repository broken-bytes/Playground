namespace PlaygroundAssembly.ECS.Core;

[AttributeUsage(AttributeTargets.Method, AllowMultiple = true)]
public sealed class EcsRequireAttribute : Attribute
{
    public Type ComponentType { get; }
    public EcsFilter.Usage Usage { get; }
    public EcsFilter.Operation Operation { get; }

    public EcsRequireAttribute(
        Type componentType,
        EcsFilter.Usage usage = EcsFilter.Usage.Read,
        EcsFilter.Operation operation = EcsFilter.Operation.And)
    {
        ComponentType = componentType;
        Usage = usage;
        Operation = operation;
    }
}
