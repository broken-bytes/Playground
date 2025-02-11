namespace Playground;

[Serializable]
internal struct SceneComponent
{
    internal string UUID;
    internal string ComponentUUID;
    internal Dictionary<string, dynamic> Components;
}
