using System;
using System.Diagnostics;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace PlaygroundEditor;

public static class EventManager {
    public enum EventType : int {
        Health = 0,
        System = 1,
        Input = 2,
        Network = 3,
    }
    
    public enum SystemEventType: int {
        Quit = 0,
        WindowResize,
        WindowMove,
        WindowFocus,
        WindowLostFocus,
    };
    
    [StructLayout(LayoutKind.Sequential)]
    public struct Event {
        public EventType Type;
        public SystemEventType SystemType;
    };
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EventCallbackDelegate(IntPtr nativeEvent);
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SubscribeToEventsDelegate(int eventType, IntPtr callback);
    
    private static SubscribeToEventsDelegate _subscribeToEvents;
    private static EventCallbackDelegate _eventCallbackDelegate;
    private static GCHandle _callbackHandle;
    
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void OnEvent(IntPtr ptr) {
        var e = Marshal.PtrToStructure<Event>(ptr);
        if (e.SystemType == SystemEventType.WindowFocus) {
            SceneViewManager.OnGotFocus();
        } else if (e.SystemType == SystemEventType.WindowLostFocus) {
            SceneViewManager.OnLostFocus();
        }
    }
    
    public static unsafe void Setup() {
        _subscribeToEvents = Marshal.GetDelegateForFunctionPointer<SubscribeToEventsDelegate>(NativeLookupTable.Lookup("Events_Subscribe"));
        var fnPtr = (IntPtr)(delegate* unmanaged[Cdecl]<IntPtr, void>)&OnEvent;
        
        _subscribeToEvents((int)EventType.System, fnPtr);
    }

    private static void ProcessWindowFocusEvent() {
        SceneViewManager.OnGotFocus();
    }
    
    private static void ProcessWindowLostFocusEvent() {
        SceneViewManager.OnLostFocus();
    }
}
