using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Loader;

namespace PlaygroundAssembly;

public static class PlaygroundAssembly
{
    [UnmanagedCallersOnly(EntryPoint = "RegisterFunction")]
    public static unsafe void RegisterFunction(byte* name, int length, IntPtr ptr)
    {
        Managed.RegisterFunction(name, length, ptr);
    }
    
    // Signature must match unmanaged expectations
    [UnmanagedCallersOnly(EntryPoint = "Startup")]
    public static unsafe int Startup(byte* location, int length)
    {
        var locationStr = System.Text.Encoding.UTF8.GetString(location, length);
        
        var loadContext = new PluginLoadContext(locationStr);
        
        var gameAssembly = loadContext.LoadFromAssemblyName(
            new AssemblyName("GameAssembly"));
        
        Console.WriteLine($"=== ALC DUMP ===");
        foreach (var alc in AssemblyLoadContext.All)
        {
            Console.WriteLine($"ALC: {alc.Name ?? "<default>"}");
            foreach (var asm in alc.Assemblies)
                Console.WriteLine($"  {asm.GetName().Name}");
        }
        
        Managed.SetupSubsystems(gameAssembly);
        Managed.SetupEcs(gameAssembly);
        return 0;
    }
}
