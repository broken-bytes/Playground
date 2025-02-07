namespace Playground;

public partial class GameObject
{
    public delegate void GameObjectCreatedDelegate(GameObject gameObject);
    public delegate void GameObjectEnabledDelegate(GameObject gameObject);
    public delegate void GameObjectDisabledDelegate(GameObject gameObject);
    public delegate void GameObjectDestroyedDelegate(GameObject gameObject);
    
    internal static event GameObjectCreatedDelegate OnGameObjectCreated;
    internal static event GameObjectEnabledDelegate OnGameObjectEnabled;
    internal static event GameObjectDisabledDelegate OnGameObjectDisabled;
    internal static event GameObjectDestroyedDelegate OnGameObjectDestroyed;
    
    internal List<Component> _components = new();
    
    internal void OnUpdate()
    {
        foreach (var component in _components)
        {
            component.OnUpdate();
        }
    }
}
