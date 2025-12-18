namespace PlaygroundAssembly.Scenes;

internal class Scene
{
    List<SceneEntity> Entities;

    public override string ToString()
    {
        return $"" +
               $"{nameof(Entities)}: {Entities}";
    }
}
