using System.Collections.Generic;
using Avalonia.Threading;

namespace PlaygroundEditor;

public static class EditorWindowManager
{
    internal static List<EditorWindow> EditorWindows = new();
    
    internal static void OnUpdate()
    {
        Dispatcher.UIThread.Post(() =>
        {
            foreach (var window in EditorWindows)
            {
                window.OnEditorUpdate();
            }
        });
    }
}
