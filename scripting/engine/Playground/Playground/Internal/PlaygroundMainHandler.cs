using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace Playground;

internal class PlaygroundMainHandler
{
    [DllImport("PlaygroundCore.dll")]
    private static extern void PlaygroundMain(ref PlaygroundConfig config);
    
    private static EmptyDelegate _shutdownDelegate;
    
    private static Thread _engineMainThread;

    static void Receive()
    {
        
    }

    static void OnStart(IntPtr window, uint width, uint height, bool isOffscreen, string assemblyPath)
    {
        var receiveFunctionTableEntry = new NativePointerTable.ReceiveFunctionTableEntryDelegate(NativePointerTable.Receive);
        var config = new PlaygroundConfig
        {
            Window = window,
            Delegate = Marshal.GetFunctionPointerForDelegate(receiveFunctionTableEntry),
            Width = width,
            Height = height,
            IsOffscreen = isOffscreen
        };
        
        EngineEnvironment.AssemblyPath = assemblyPath;
            
        SceneManager.OnStart();
            
        PlaygroundMain(ref config);
    }

    static void OnUpdate()
    {
        Input.OnUpdate();
        Renderer.OnPreFrame();
        SceneManager.OnUpdate();
        Renderer.OnUpdate();
        Renderer.OnPostFrame();
        Time.OnTick();
    }

    static void OnDestroy()
    {
        SceneManager.OnDestroy();

        Marshal.GetDelegateForFunctionPointer<EmptyDelegate>(NativePointerTable.GetPointer("Playground_Shutdown"))();
    }
}
