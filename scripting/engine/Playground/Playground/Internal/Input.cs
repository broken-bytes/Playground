using System.Runtime.InteropServices;

namespace Playground;

internal static class Input
{
    private static EmptyDelegate _updatePtr;
    
    static Input()
    {
        _updatePtr = Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Input_Update"));
    }
    
    internal static void OnUpdate()
    {
        _updatePtr();
    }
}
