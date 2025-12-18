namespace PlaygroundAssembly.Scenes;

public class Scene
{
    public List<SceneEntity> Entities { get; set; } = new();

    public override string ToString()
    {
        return $"" +
               $"{nameof(Entities)}: {Entities}";
    }
}
