using System.Runtime.InteropServices;
using Playground.Internal;

namespace Playground;

public partial class GameObject
{
    public delegate void GameObjectCreatedDelegate(GameObject gameObject);
    public delegate void GameObjectEnabledDelegate(GameObject gameObject);
    public delegate void GameObjectDisabledDelegate(GameObject gameObject);
    public delegate void GameObjectDestroyedDelegate(GameObject gameObject);
    
    internal static event GameObjectCreatedDelegate? OnGameObjectCreated;
    internal static event GameObjectEnabledDelegate? OnGameObjectEnabled;
    internal static event GameObjectDisabledDelegate? OnGameObjectDisabled;
    internal static event GameObjectDestroyedDelegate? OnGameObjectDestroyed;
    
    private static UpdateTransformDelegate _updateTransformPtr;
    
    private static uint _idCounter;
    
    internal List<Component> _components = new();
    private uint _id;
    private Transform _transform;
    
    static GameObject()
    {
    }
    
    private GameObject()
    {
        _id = _idCounter++;
        _transform = new();
    }
    
    internal void OnUpdate()
    {
        foreach (var component in _components)
        {
            component.OnUpdate();
        }
    }
}
