using System.Diagnostics.CodeAnalysis;
using Playground.Core.Ecs;

namespace PlaygroundAssembly.ECS;

public struct Entity : IEquatable<Entity>
{
    internal ulong Id { get; set; }
    
    internal Entity(ulong id) => Id = id;

    public void AddComponent<T>() where T : unmanaged
    {
        EcsApi.AddComponent<T>(Id);
    }

    public void SetComponent<T>(T component) where T : unmanaged
    {
        EcsApi.SetComponent(Id, component);
    }

    public void RemoveComponent<T>() where T : unmanaged
    {
        EcsApi.RemoveComponent<T>(Id);
    }

    public Entity With<T>(T component) where T : unmanaged
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
