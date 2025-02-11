namespace Playground;

[Serializable]
public class SceneGameObject
{
    public string Name;
    public string UUID;
    public string Tag;
    public int Layer;
    public List<SceneComponent> Components;
}
