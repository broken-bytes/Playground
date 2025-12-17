namespace Playground.Core.Ecs;

public readonly ref struct EcsContext(IntPtr iterator, ulong systemId)
{
    public bool Has<T>()
    {
        if (EcsApi.RegisteredComponents.TryGetValue(typeof(T), out var value))
        {
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} is not registered.");
        }
        
        // TODO: Get components via iterator iterator

        return true;
    }
    
    public ComponentView<T> Get<T>()
    {
        if (EcsApi.RegisteredComponents.TryGetValue(typeof(T), out var value))
        {
            throw new InvalidOperationException(
                $"Component {typeof(T).Name} is not registered.");
        }

        // TODO: Get components via iterator iterator

        return new ComponentView<T>([]);
    }
}
