using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Avalonia.Controls;
using Avalonia.Platform;

namespace PlaygroundEditor;

public class NativeEngineEmbedHost : NativeControlHost
{
    public nint _handle;
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void LookupTableDelegate(IntPtr name, IntPtr function);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ScriptStartupCallback();
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void ScriptingStartup(IntPtr lookupTableDelegate, IntPtr scriptStartupCallback);
    
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct PlaygroundConfig {
        public IntPtr Delegate;
        public IntPtr StartupCallback;
        public UInt32 Width;
        public UInt32 Height;
        [MarshalAs(UnmanagedType.U1)]
        public bool Fullscreen;
        [MarshalAs(UnmanagedType.LPStr)]
        public string WindowName;
        [MarshalAs(UnmanagedType.LPStr)]
        public string Path;
        public IntPtr WindowHandle;
    }
    
    [DllImport("PlaygroundCore.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern ushort PlaygroundCoreMain(ref PlaygroundConfig config);
    
    [DllImport("Playground.dll", CallingConvention = CallingConvention.Cdecl)]
    public static extern ushort PlaygroundMain(ScriptingStartup startup);
    
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
