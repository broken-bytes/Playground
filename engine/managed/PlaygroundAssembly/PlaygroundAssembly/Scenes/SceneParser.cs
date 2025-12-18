using System.Text.Json;

namespace PlaygroundAssembly.Scenes;

internal static class SceneParser
{
    internal static Scene Parse(string data)
    {
        var scene = JsonSerializer.Deserialize<Scene>(data);

        // TODO: Add proper exception
        return scene ?? throw new Exception("Failed to parse scene");
    }
}
