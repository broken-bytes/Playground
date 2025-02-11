using System.Runtime.InteropServices;

namespace Playground;

internal static class NativePointerTable
{
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    internal delegate void ReceiveFunctionTableEntryDelegate(string name, IntPtr pointer);
    
    static Dictionary<string, IntPtr> _pointerTable = new();

    internal static unsafe void Receive(string name, IntPtr pointer)
    {
        // Turn the c string into a string
        _pointerTable.Add(name, pointer);
    }

    internal static IntPtr GetPointer(string name)
    {
        return _pointerTable[name];
    }
}
