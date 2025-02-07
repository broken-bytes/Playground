using System.Reflection;
using System.Runtime.InteropServices;
using System.Threading;

namespace Playground.Internal;

internal class PlaygroundMainHandler
{
    [DllImport("user32.dll")]
    private static extern IntPtr GetActiveWindow ();
    
    [DllImport("PlaygroundCore.dll")]
    private static extern void PlaygroundMain(ref PlaygroundConfig config);
    
    private static Thread _engineMainThread;

    static void Receive()
    {
        
    }

    static void OnStart(IntPtr window)
    {
        _engineMainThread = new Thread(() =>
        {
            var receiveFunctionTableEntry = new NativePointerTable.ReceiveFunctionTableEntryDelegate(NativePointerTable.Receive);
            var config = new PlaygroundConfig
            {
                Window = window,
                Delegate = Marshal.GetFunctionPointerForDelegate(receiveFunctionTableEntry),
                Width = 1024,
                Height = 768
            };
            
            SceneManager.OnStart();
            
            PlaygroundMain(ref config);
            
            var renderer = new Renderer();
            
            while (true)
            {
                renderer.OnPreFrame();
                SceneManager.OnUpdate();
                renderer.OnUpdate(Time.DeltaTime);
                renderer.OnPostFrame();
                Time.OnTick();
            } 
        });
        
        _engineMainThread.Start();
    }
}
