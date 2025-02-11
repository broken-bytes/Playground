using System.Runtime.InteropServices;

namespace Playground;

internal class Renderer
{
    private static EmptyDelegate _preFramePtr;
    private static UpdateDelegate _updatePtr;
    private static EmptyDelegate _postFramePtr;
    private static ReadBackBufferDelegate _readBackBufferPtr;
    
    static Renderer()
    {
        _preFramePtr = Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Rendering_PreFrame"));
        _updatePtr = Marshal.GetDelegateForFunctionPointer<UpdateDelegate>(NativePointerTable.GetPointer("Rendering_Update"));
        _postFramePtr = Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Rendering_PostFrame"));
        _readBackBufferPtr = Marshal.GetDelegateForFunctionPointer<ReadBackBufferDelegate>(NativePointerTable.GetPointer("Rendering_ReadBackBuffer"));
    }

    internal static void OnPreFrame()
    {
        _preFramePtr();
    }
    
    internal static void OnUpdate()
    {
        _updatePtr(Time.DeltaTime);
    }

    internal static void OnPostFrame()
    {
        _postFramePtr();
    }
    
    internal static uint ReadBackBuffer(IntPtr readbackBuffer)
    {
        uint bufferSize = _readBackBufferPtr(readbackBuffer);

        return bufferSize;
    }
}
