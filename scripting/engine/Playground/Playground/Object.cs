namespace Playground;

public class Object {
    public static void Destroy(Object objc) {
        if (objc is GameObject go) {
            SceneManager.Delete(go);
        }
    }
}
