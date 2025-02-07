namespace Playground;

public partial class GameObject
{
    public GameObject? Parent;
    public string Name;
    
    public GameObject(string? name = null, GameObject? parent = null)
    {
        Name = name ?? "Unnamed GameObject";
        Parent = parent;
        OnGameObjectCreated?.Invoke(this);
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
            return existing;
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
        component.Dispose();
    }
}
