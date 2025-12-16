namespace PlaygroundAssembly.ECS.Core;

public readonly ref struct EcsContext
{
    private readonly Dictionary<ulong, ComponentView> _views;
    private readonly HashSet<ulong> _declared;

    internal EcsContext(
        Dictionary<ulong, ComponentView> views,
        HashSet<ulong> declared)
    {
        _views = views;
        _declared = declared;
    }

    /// <summary>
    /// Returns true if the system declared this component
    /// AND the current archetype provides it.
    /// </summary>
    public bool Has<T>()
    {
        if (EcsApi.RegisteredComponents.TryGetValue(typeof(T), out var value))
        {
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} is not registered.");
        }
        
        return _declared.Contains(value) && _views.ContainsKey(value);
    }

    /// <summary>
    /// Gets the component view for this system.
    /// Throws if access is invalid.
    /// </summary>
    public ComponentView<T> Get<T>()
    {
        if (EcsApi.RegisteredComponents.TryGetValue(typeof(T), out var value))
        {
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} is not registered.");
        }

        if (!_declared.Contains(value))
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} was not declared by this system.");

        if (!_views.TryGetValue(value, out var view))
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} is not present on this archetype.");

        return (ComponentView<T>)view;
    }
}
