using System.Runtime.InteropServices;
using Playground.Internal;

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
