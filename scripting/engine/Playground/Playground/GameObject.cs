namespace Playground;

public partial class GameObject
{
    public GameObject? Parent;
    public Transform Transform;
    public string Name;
    public int Layer;
    public string? Tag;
    
    public GameObject(string? name = null, GameObject? parent = null) : this()
    {
        Name = name ?? "Unnamed GameObject";
        Parent = parent;
        OnGameObjectCreated?.Invoke(this);
        Layer = 0;
        Tag = null;
    }

    ~GameObject()
    {
        OnGameObjectDestroyed?.Invoke(this);
    }
    
    public T Attach<T>() where T : Component, new()
    {
        var existing = _components.OfType<T>().FirstOrDefault();

        if (existing != null)
        {
            throw new ComponentAlreadyAttachedException<T>(this);
        }
        
        T component = new T
        {
            GameObject = this
        };
        
        _components.Add(component);
        
        component.GameObject = this;
        
        return component;
    }

    public void Remove<T>() where T : Component
    {
        var component = _components.OfType<T>().FirstOrDefault();

        if (component == null)
        {
            return;
        }
        
        _components.Remove(component);
    }

    public T Retrieve<T>() where T : Component
    {
        var component = _components.OfType<T>().FirstOrDefault();

        if (component == null)
        {
            throw new ComponentNotFoundException<T>(this);
        }

        return component;
    }

    public static GameObject? FindGameObject(string name) => 
        SceneManager.SceneObjects.FirstOrDefault(objc => objc.Name == name);

    public static void Destroy(GameObject gameObject)
    {
        SceneManager.Delete(gameObject);
    }
    
    public void OnDestroy()
    {
        _isDestroyed = true;
        foreach (var comp in _components)
        {
            comp._isDestroyed = true;
            comp.OnDestroy();
        }
    }
}
