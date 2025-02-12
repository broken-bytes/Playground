using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;

namespace Playground;

public partial class GameObject : Object
{
    public GameObject? Parent;
    public Transform Transform;
    public string Name;
    public string UniqueIdentifier { get; internal set; }
    public int Layer;
    public string? Tag;
    
    public ReadOnlyCollection<GameObject> Children => new(_children);
    
    public GameObject(string? name = null, GameObject? parent = null) : this()
    {
        Name = name ?? "Unnamed GameObject";
        UniqueIdentifier = Guid.NewGuid().ToString();
        Parent = parent;
        OnGameObjectCreated?.Invoke(this);
        Layer = 0;
        Tag = null;
    }

    ~GameObject()
    {
        OnGameObjectDestroyed?.Invoke(this);
    }

    public void Attach<T>() where T : Component, new() {
        if (TryRetrieve<T>() is { } comp) {
            throw new ComponentAlreadyAttachedException<T>(this);
        }
        
        _components.Add(
            new T { GameObject = this }
        );
    }
    
    public T AttachOrRetrieve<T>() where T : Component, new() {
        
        if (TryRetrieve<T>() is { } comp)
        {
            return comp;
        }
        
        var component = new T
        {
            GameObject = this
        };
        
        _components.Add(component);
        
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

    public T Retrieve<T>(ComponentRetrieveScope scope = ComponentRetrieveScope.Self) where T : Component {
        if (scope is ComponentRetrieveScope.Parent) {
            if (Parent == null) {
                throw new GameObjectHasNoParentException<T>(this);
            }
            
            return Parent.Retrieve<T>();
        }
        
        var component = _components.OfType<T>().FirstOrDefault();

        if (component == null) {
            if (scope is ComponentRetrieveScope.SelfAndParent or ComponentRetrieveScope.SelfAndChildrenAndParent) {
                if (Parent?.TryRetrieve<T>() is { } parentComp) {
                    return parentComp;
                }
            }
            if (scope is ComponentRetrieveScope.SelfAndChildren or ComponentRetrieveScope.SelfAndChildrenAndParent) {
                foreach (var objc in _children) {
                    if (objc.TryRetrieve<T>(scope) is { } comp) {
                        return comp;
                    }
                }
            }

            throw new ComponentNotFoundException<T>(this);
        }

        return component;
    }

    public T? TryRetrieve<T>(ComponentRetrieveScope scope = ComponentRetrieveScope.Self) where T : Component {
        try {
            return Retrieve<T>(scope);
        }
        catch {
            return null;
        }
    }

    public static GameObject? FindGameObject(string name) => 
        SceneManager.SceneObjects.FirstOrDefault(objc => objc.Name == name);
    
    public void OnDestroy()
    {
        _isDestroyed = true;
        foreach (var comp in _components)
        {
            comp._isDestroyed = true;
            comp.OnDestroy();
        }
    }

    public void SetParent(GameObject parent)
    {
        SceneManager.SceneObjects.Remove(this);
        Parent = parent;
        parent._children.Add(this);
    }
}
