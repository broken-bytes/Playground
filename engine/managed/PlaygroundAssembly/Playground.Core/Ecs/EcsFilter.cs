namespace Playground.Core.Ecs;

public struct EcsFilter
{ 
    public ulong Id;
    public EcsUsage ParamUsage;
    public EcsOperation ParamOperation;
    
    internal static EcsFilter[] BuildFilters(
        EcsQueryItemAttribute[] items
    )
    {
        if (items.Length == 0)
        {
            return Array.Empty<EcsFilter>();
        }
        var filters = new EcsFilter[items.Count()];
        int i = 0;

        foreach (var item in items)
        {
            filters[i++] = new EcsFilter
            {
                Id = EcsApi.RegisteredComponents.TryGetValue(item.ComponentType, out var value) ? value : throw new Exception(),
                ParamUsage = item.Usage,
                ParamOperation = item.Operation
            };
        }

        return filters;
    }
}

