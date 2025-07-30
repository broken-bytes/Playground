using System.Collections.Generic;
using System.IO;

namespace PlaygroundEditor;

public static class SceneManager
{
    internal static object? SceneData;


    public static void Setup() {
        
    }

    internal static void LoadScene(string sceneName)
    {
        var sceneData = File.ReadAllText(EditorEnvironment.ProjectPath + $"/content/scenes/{sceneName}.pscn");
        // TODO: Load scene
    }

    internal static void CommitScene()
    {
        
    }

    internal static void ResetScene()
    {
        
    }
}
