using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace PlaygroundEditor;

public static class EventManager {
    public enum EventType {
        Health,
        System,
        Input,
        Network,
    };
    
    public class Event {
        public EventType Type;
    };
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EventCallbackDelegate(Event nativeEvent);
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SubscribeToEventsDelegate(EventCallbackDelegate @delegate);
    
    private static SubscribeToEventsDelegate _subscribeToEvents;
    
    public static void Setup() {
        _subscribeToEvents = Marshal.GetDelegateForFunctionPointer<SubscribeToEventsDelegate>(NativeLookupTable.Lookup("Events_Subscribe"));

        _subscribeToEvents(
            (e) => {
                Debug.WriteLine("RECEIVED EVENT");
                if (e.Type == EventType.System) {
                    Debug.WriteLine("System event");
                }
            });
    }

    private static void ProcessWindowFocusEvent() {
        SceneViewManager.OnGotFocus();
    }
    
    private static void ProcessWindowLostFocusEvent() {
        SceneViewManager.OnLostFocus();
    }
}
