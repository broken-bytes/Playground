using System.Runtime.InteropServices;

namespace Playground.Internal;

internal class Renderer
{
    private EmptyDelegate _preFramePtr;
    private UpdateDelegate _updatePtr;
    private EmptyDelegate _postFramePtr;
    
    internal Renderer()
    {
        _preFramePtr = Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Rendering_PreFrame"));
        _updatePtr = Marshal.GetDelegateForFunctionPointer<UpdateDelegate>(NativePointerTable.GetPointer("Rendering_Update"));
        _postFramePtr = Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Rendering_PostFrame"));
    }

    internal void OnPreFrame()
    {
        _preFramePtr();
    }
    
    internal void OnUpdate(double deltaTime)
    {
        _updatePtr(deltaTime);
    }

    internal void OnPostFrame()
    {
        _postFramePtr();
    }
    
}
