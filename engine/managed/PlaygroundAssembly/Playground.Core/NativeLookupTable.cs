using System.Text;

namespace Playground.Core;

internal static unsafe class NativeLookupTable
{
    private static readonly Dictionary<string, IntPtr> Table = new();

    internal static void* GetFunctionPointer(string name)
    {
        return (void*)Table[name];
    }

    internal static void AddEntry(byte* key, int length, IntPtr ptr)
    {
        var strName = Encoding.UTF8.GetString(key, length);
        Table[strName] = ptr;
    }
}
