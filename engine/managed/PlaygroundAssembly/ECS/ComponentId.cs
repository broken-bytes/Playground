namespace PlaygroundAssembly.ECS;

static class ComponentId<T> where T : unmanaged, IComponent
{
    internal static readonly ulong Value = Register();
    
    private static unsafe ulong Register()
    {
        return ECSApi.RegisterComponent<T>();
    }
}
