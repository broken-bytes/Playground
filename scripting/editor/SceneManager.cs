using System.Collections.Generic;
using System.IO;
using Playground;

namespace PlaygroundEditor;

public static class SceneManager
{
    internal static object? SceneData;
    
    static SceneManager()
    {
        
    }

    internal static void LoadScene(string sceneName)
    {
        var sceneData = File.ReadAllText(EditorEnvironment.ProjectPath + $"/content/scenes/{sceneName}.pscn");
        Playground.SceneManager.LoadScene(sceneName);
    }

    internal static void CommitScene()
    {
        
    }

    internal static void ResetScene()
    {
        
    }
}
