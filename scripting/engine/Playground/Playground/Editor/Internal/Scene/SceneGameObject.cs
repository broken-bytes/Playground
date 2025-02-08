namespace Playground.Editor;

[Serializable]
internal class SceneGameObject
{
    internal string Name;
    internal string UUID;
    internal string Tag;
    internal int Layer;
    internal List<SceneComponent> Components;
}
