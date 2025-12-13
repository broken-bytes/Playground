using System.Diagnostics.CodeAnalysis;

namespace PlaygroundAssembly.ECS;

public struct Entity : IEquatable<Entity>
{
    internal ulong Id { get; set; }
    
    internal Entity(ulong id) => Id = id;

    public void AddComponent<T>() where T : unmanaged, IComponent
    {
        unsafe
        {
            ECSApi.AddComponentPtr(Id, ECSApi.RegisterComponent<T>());
        }
    }

    public void SetComponent<T>(T component) where T : unmanaged, IComponent
    {
        unsafe
        {
            ECSApi.SetComponentPtr(Id, ECSApi.RegisterComponent<T>(), &component);
        }
    }

    public void RemoveComponent<T>() where T : unmanaged, IComponent
    {
        unsafe
        {
            ECSApi.RemoveComponentPtr(Id, ECSApi.RegisterComponent<T>());
        }
    }

    public Entity With<T>(T component) where T : unmanaged, IComponent
    {
        AddComponent<T>();
        SetComponent(component);

        return this;
    }

    public override bool Equals([NotNullWhen(true)] object? obj)
    {
        return obj is Entity entity && Id == entity.Id;
    }

    public bool Equals(Entity other)
    {
        return Id == other.Id;
    }

    public override int GetHashCode()
    {
        return Id.GetHashCode();
    }

    public static bool operator ==(Entity left, Entity right)
    {
        return left.Equals(right);
    }

    public static bool operator !=(Entity left, Entity right)
    {
        return !(left == right);
    }
}
