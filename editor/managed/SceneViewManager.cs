using System.Runtime.InteropServices;

namespace PlaygroundEditor;

public static class SceneViewManager {
    public delegate void SetCapturesInputDelegate(bool capture);
    
    private static SetCapturesInputDelegate _setCapturesInput;
    
    public static void Setup() {
        _setCapturesInput = Marshal.GetDelegateForFunctionPointer<SetCapturesInputDelegate>(NativeLookupTable.Lookup("Input_SetCapturesInput"));
    }

    public static void OnGotFocus() {
        _setCapturesInput?.Invoke(true);
    }

    public static void OnLostFocus() {
        _setCapturesInput?.Invoke(false);
    }
}
