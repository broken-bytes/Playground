using System.Linq;
using System.Reflection;

namespace PlaygroundEditor;

using Playground;

internal static class EditorLoop
{
    private static MethodInfo? _rendererPreUpdate;
    private static MethodInfo? _renderUpdate;
    private static MethodInfo? _renderPostUpdate;
    private static MethodInfo? _inputUpdate;
    private static MethodInfo? _sceneUpdate;
    private static MethodInfo? _timerTick;
    
    static EditorLoop()
    {
        var assembly = Assembly.Load("Playground");
        var types = assembly.GetTypes();
        
        var renderer = types.FirstOrDefault(type => type.Name.Equals("Renderer"));
        _rendererPreUpdate = renderer?.GetMethod("OnPreFrame", BindingFlags.Static | BindingFlags.NonPublic);
        _renderUpdate = renderer?.GetMethod("OnUpdate", BindingFlags.Static | BindingFlags.NonPublic);
        _renderPostUpdate = renderer?.GetMethod("OnPostFrame", BindingFlags.Static | BindingFlags.NonPublic);
        
        var input = types.FirstOrDefault(type => type.Name.Equals("Input"));
        _inputUpdate = input?.GetMethod("OnUpdate", BindingFlags.Static | BindingFlags.NonPublic);
        
        var sceneManager = types.FirstOrDefault(type => type.Name.Equals("SceneManager"));
        _sceneUpdate = sceneManager?.GetMethod("OnUpdate", BindingFlags.Static | BindingFlags.NonPublic);
        
        var timer = types.FirstOrDefault(type => type.Name.Equals("Time"));
        _timerTick = timer?.GetMethod("OnTick", BindingFlags.Static | BindingFlags.NonPublic);
    }
    
    internal static void OnUpdate()
    {
        if (EditorEnvironment.IsPlayMode)
        {
            _inputUpdate?.Invoke(null, null);
        }
        
        _rendererPreUpdate?.Invoke(null, null);

        if (EditorEnvironment.IsPlayMode)
        {
            _sceneUpdate?.Invoke(null, null);
        }
        
        _renderUpdate?.Invoke(null, null);
        _renderPostUpdate?.Invoke(null, null);
        _timerTick?.Invoke(null, null);
    }
}
