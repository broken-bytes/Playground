using System.Text.Json;

namespace PlaygroundAssembly.Scenes;

internal struct SceneComponent
{
    internal string TypeName;
    internal Dictionary<string, JsonElement> Properties;
}
