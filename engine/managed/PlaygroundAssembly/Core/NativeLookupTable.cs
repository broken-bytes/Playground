namespace PlaygroundAssembly.Core;

internal static unsafe class NativeLookupTable
{
    private static readonly Dictionary<string, IntPtr> Table = new();

    internal static void* GetFunctionPointer(string name)
    {
        return (void*)Table[name];
    }

    internal static void AddEntry(string key, IntPtr ptr)
    {
        Table[key] = ptr;
    }
}
