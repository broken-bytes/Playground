using System.Diagnostics;
using System.Runtime.InteropServices;
using Avalonia.Controls;
using Avalonia.Platform;

namespace PlaygroundEditor;

public class NativeEngineEmbedHost : NativeControlHost
{
    public nint _handle;
    
    protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
    {
        var piHandle = base.CreateNativeControlCore(parent);
        _handle = piHandle.Handle;
        
        return new PlatformHandle(_handle, null);
    }

    protected override void DestroyNativeControlCore(IPlatformHandle control)
    {
        base.DestroyNativeControlCore(control);
    }
}
