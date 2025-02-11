using System.Runtime.InteropServices;

namespace Playground;

public partial class GameObject
{
    public delegate void GameObjectCreatedDelegate(GameObject gameObject);
    public delegate void GameObjectEnabledDelegate(GameObject gameObject);
    public delegate void GameObjectDisabledDelegate(GameObject gameObject);
    public delegate void GameObjectDestroyedDelegate(GameObject gameObject);
    
    public static event GameObjectCreatedDelegate? OnGameObjectCreated;
    public static event GameObjectEnabledDelegate? OnGameObjectEnabled;
    public static event GameObjectDisabledDelegate? OnGameObjectDisabled;
    public static event GameObjectDestroyedDelegate? OnGameObjectDestroyed;
    
    internal List<Component> _components = new();
    internal bool _isDestroyed;
    private List<GameObject> _children = new();
    private static uint _idCounter;
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
