using System.Diagnostics.CodeAnalysis;
using System.Runtime.InteropServices;
using Playground.Core.Ecs;

namespace PlaygroundAssembly.ECS;

public struct Entity : IEquatable<Entity>
{
    internal ulong Id { get; set; }
    
    internal Entity(ulong id) => Id = id;

    public Entity(string name)
    {
        Id = EcsApi.CreateEntity(name);
    }

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
    
    internal Entity With(Type componentType, object component)
    {
        if (component.GetType() != componentType)
        {
            throw new ArgumentException("Component type mismatch");
        }

        SetComponent(component);

        return this;
    }
    
    internal void SetComponent(object component)
    {
        if (component == null)
        {
            throw new ArgumentNullException(nameof(component));
        }

        var type = component.GetType();

        if (!type.IsValueType)
        {
            throw new ArgumentException("Component must be a struct");
        }
        
        SetComponentBoxed(type, component);
    }
    
    private void SetComponentBoxed(Type type, object boxed)
    {
        // Pin boxed struct and copy raw bytes
        unsafe
        {
            var size = Marshal.SizeOf(type);
            var handle = GCHandle.Alloc(boxed, GCHandleType.Pinned);

            try
            {
                void* ptr = handle.AddrOfPinnedObject().ToPointer();
                EcsApi.SetComponent(Id, type, boxed);
            }
            finally
            {
                handle.Free();
            }
        }
    }
}
