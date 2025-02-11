namespace Playground;

[Serializable]
public struct SceneComponent
{
    [Serializable]
    public struct Field
    {
        public string Name;
        public object Value;
    }
    public string TypeName;
    public List<Field> Fields;
}
