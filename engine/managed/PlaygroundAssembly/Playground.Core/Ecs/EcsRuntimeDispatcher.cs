using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Playground.Core.Ecs;

public static unsafe class EcsRuntimeDispatcher
{
    private static readonly Dictionary<ulong, Action<EcsContext, double>> Systems = new();
    
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void Invoke(IntPtr ctx)
    {
        Dispatch(ctx);
    }

    public static void Register(ulong id, Action<EcsContext, double> system)
    {
        Systems[id] = system;
    }
    
    private static void Dispatch(IntPtr ctrPtr)
    {
        var systemId = EcsApi.GetIteratorSystem(ctrPtr);
        var ctx = new EcsContext(ctrPtr, Systems.TryGetValue(systemId, out var value) ? systemId : throw new InvalidOperationException());
        
        Systems[systemId](ctx, 0);
    }
}
