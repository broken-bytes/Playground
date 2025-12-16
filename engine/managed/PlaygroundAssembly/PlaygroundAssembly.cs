using System.Runtime.InteropServices;

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
    public static int Startup()
    {
        Managed.SetupSubsystems();
        Managed.SetupEcs();
        return 0;
    }
}
