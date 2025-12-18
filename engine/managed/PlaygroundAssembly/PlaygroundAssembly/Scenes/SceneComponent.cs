using System.Text.Json;

namespace PlaygroundAssembly.Scenes;

public sealed class SceneComponentTypeInfo
{
    public string Assembly { get; set; }
    public string Name { get; set; }
}

public sealed class SceneComponent
{
    public SceneComponentTypeInfo Type { get; set; }
    public Dictionary<string, JsonElement> Properties { get; set; } = new();
}
