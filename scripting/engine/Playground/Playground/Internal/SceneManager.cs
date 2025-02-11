using System.Diagnostics;

namespace Playground;

public static partial class SceneManager
{
    internal static double _fixedTimeStep;
    internal static List<GameObject> ToBeDestroyedObjects = new();
    
    internal static void OnStart()
    {
        GameObject.OnGameObjectCreated += (objc) =>
        {
            SceneObjects.Add(objc);
        };

        GameObject.OnGameObjectDestroyed += (objc) =>
        {
            SceneObjects.Remove(objc);
        };
    }

    internal static void OnDestroy()
    {
        foreach (var objc in SceneObjects)
        {
            foreach (var comp in objc._components)
            {
                comp.OnDestroy();
            }
        }
        
        SceneObjects.Clear();
    }
    
    internal static void OnUpdate()
    {
        // Update script code on each gameobject
        foreach (var objc in SceneObjects)
        {
            objc.OnUpdate();
        }

        foreach (var objc in ToBeDestroyedObjects)
        {
            SceneObjects.Remove(objc);
            objc.OnDestroy();
        }
        
        ToBeDestroyedObjects.Clear();
    }

    internal static void Delete(GameObject objc)
    {
        ToBeDestroyedObjects.Add(objc);
    }
}
