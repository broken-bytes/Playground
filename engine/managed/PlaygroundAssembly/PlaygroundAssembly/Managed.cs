using System.Reflection;
using System.Runtime.Loader;
using Playground.Core;
using Playground.Core.Ecs;
using Playground.Core.Logging;
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

    internal static void SetupSubsystems(System.Reflection.Assembly assembly)
    {
        AssemblyLoadContext.Default.Resolving += (context, assemblyName) =>
        {
            if (assemblyName.Name == "PlaygroundAssembly")
            {
                return System.Reflection.Assembly.Load(assemblyName);
            }

            return null;
        };
        
        LoggerApi.Setup();
        EcsApi.Setup();
    }
    
    internal static void SetupEcs(System.Reflection.Assembly assembly)
    {
        SetupEcsComponents(assembly);
        SetupEcsSystems(assembly);
    }

    private static unsafe void SetupEcsSystems(System.Reflection.Assembly assembly)
    {
        var currentAssembly = System.Reflection.Assembly.GetExecutingAssembly();
        var types = currentAssembly.GetTypes().Concat(assembly.GetTypes()).ToArray();
        
        Logger.Info($"Types: {types.Length}");

        foreach (var type in types)
        {
            if (!type.IsClass)
            {
                continue;
            }

            foreach (
                var method in type.GetMethods(
                    BindingFlags.Static |
                    BindingFlags.Public |
                    BindingFlags.NonPublic
                )
            )
            {
                var systemAttr = method.GetCustomAttribute<EcsSystemAttribute>();
                if (systemAttr == null)
                {
                    continue;
                }

                if (method.IsStatic == false)
                {
                    Logger.Error($"System {type.Name} is not static");
                    continue;
                }

                if (method.IsGenericMethod)
                {
                    Logger.Error($"System {type.Name} is generic");
                    continue;
                }

                if (method.IsAbstract)
                {
                    Logger.Error($"System {type.Name} is abstract");
                    continue;
                }

                if (method.IsPrivate)
                {
                    Logger.Error($"System {type.Name} is private");
                    continue;
                }

                if (method.Name != "Execute")
                {
                    Logger.Error($"System {type.Name} has invalid name");
                    continue;
                }

                if (method.ReturnType != typeof(void))
                {
                    Logger.Error($"System {type.Name} has invalid return type");
                    continue;
                }

                if (method.GetParameters().Length != 2)
                {
                    Logger.Error($"System {type.Name} has invalid parameter count");
                    continue;
                }

                if (method.GetParameters()[0].ParameterType != typeof(EcsContext) || method.GetParameters()[1].ParameterType != typeof(double))
                {
                    Logger.Error($"System {type.Name} has invalid parameter type");
                    continue;
                }

                var queries = method.GetCustomAttributes<EcsQueryItemAttribute>().ToArray();

                var filters = EcsFilter.BuildFilters(queries);
                
                Logger.Info($"Registering system {type.Name}");

                var id = EcsApi.CreateSystem(type.Name, filters, systemAttr.IsMultiThreaded, &EcsRuntimeDispatcher.Invoke);
                
                var del = (Action<EcsContext, double>)method.CreateDelegate(typeof(Action<EcsContext, double>));
                
                EcsRuntimeDispatcher.Register(id, del);
            }
        }
    }
    
    private static void SetupEcsComponents(System.Reflection.Assembly assembly)
    {
        var currentAssembly = System.Reflection.Assembly.GetExecutingAssembly();

        foreach (var type in assembly.GetTypes())
        {
            Logger.Info($"Type: {type.Name}");
        }
        
        var types = currentAssembly.GetTypes().Concat(assembly.GetTypes()).ToArray();
        
        Logger.Info($"Types: {types.Length}");
        
        types
            .Where(t => t.GetCustomAttribute<EcsComponentAttribute>() != null)
            .ToList()
            .ForEach(component =>
            {
                Logger.Info($"Registering component {component.Name}");
                EcsApi.RegisterComponent(component);
            });
    }
}
