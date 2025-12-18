namespace PlaygroundAssembly.Scenes;

public sealed class SceneEntity
{
    public string Name { get; set; }
    public List<SceneComponent> Components { get; set; } = new();
}
