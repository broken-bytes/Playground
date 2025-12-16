using System.Reflection;
using PlaygroundAssembly.Core;
using PlaygroundAssembly.ECS;
using PlaygroundAssembly.ECS.Core;

namespace PlaygroundAssembly;

internal static class Managed
{
    internal static unsafe void RegisterFunction(byte* name, int length, IntPtr ptr)
    {
        if (length <= 0 || ptr == IntPtr.Zero)
        {
            Console.WriteLine("Invalid function registration");
            return;
        }
        
        NativeLookupTable.AddEntry(name, length, ptr);
    }

    internal static void SetupSubsystems()
    {
        LoggerApi.Setup();
        EcsApi.Setup();
    }
    
    internal static void SetupEcs()
    {
        SetupEcsComponents();
        SetupEcsSystems();
    }

    private static void SetupEcsSystems()
    {
        var assembly = System.Reflection.Assembly.GetExecutingAssembly();

        assembly
            .GetTypes()
            .Where(t => t.GetCustomAttribute<EcsSystemAttribute>() != null)
            .ToList()
            .ForEach(system =>
            {
                
            });
    }
    
    private static void SetupEcsComponents()
    {
        var assembly = System.Reflection.Assembly.GetExecutingAssembly();
        
        assembly
            .GetTypes()
            .Where(t => t.GetCustomAttribute<EcsComponentAttribute>() != null)
            .ToList()
            .ForEach(component =>
            {
                Logger.Info($"Registering component {component.Name}");
                EcsApi.RegisterComponent(component);
            });
    }
}
