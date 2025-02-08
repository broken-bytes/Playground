using System.Diagnostics;

namespace Playground.Internal;

internal static class SceneManager
{
    internal static double _fixedTimeStep;
    internal static List<GameObject> SceneObjects = new();
    
    internal static void OnStart()
    {
        Debug.Print("SceneManager: OnStart");
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
    }
}
