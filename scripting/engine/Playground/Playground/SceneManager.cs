namespace Playground;

public static partial class SceneManager
{
    public static List<GameObject> SceneObjects { get; internal set; } = [];

    public static void LoadScene(string sceneName)
    {
        var test = new GameObject("TEST");
        test.Transform = new Transform
        {
            Position = new Vector3(3, 0 , 0),
            Rotation = Quaternion.Identity,
            Scale = Vector3.One
        };
        var objc = new GameObject("Foo");
        new GameObject("Bar").SetParent(objc);
    }
}
