using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using PlaygroundAssembly.Core;

namespace PlaygroundAssembly.ECS;

using unsafe IteratorDelegate = delegate* unmanaged[Cdecl]<IntPtr, void>;

[StructLayout(LayoutKind.Sequential)]
struct AlignOfHelper<T> where T : unmanaged
{
    public byte dummy;
    public T data;
}

internal static class ECSApi
{
    internal static unsafe delegate* unmanaged[Cdecl]<void*, void> IteratorDelegate;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, ulong> CreateEntityPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, void> DestroyEntityPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, void> SetParentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong> GetParentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, ulong> GetEntityByNamePtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, ulong, ulong, ulong> RegisterComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, void> AddComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, void*, void> SetComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, void*> GetComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, bool> HasComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<ulong, ulong, void> RemoveComponentPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, void*, ulong, bool, IteratorDelegate, ulong> CreateSystemPtr;
    
    private static readonly Dictionary<Type, ulong> RegisteredComponents = new();
    
    internal static unsafe void Setup()
    {
        CreateEntityPtr =
            (delegate* unmanaged[Cdecl]<byte*, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_CreateEntity");
        
        DestroyEntityPtr =
            (delegate* unmanaged[Cdecl]<ulong, void>)
            NativeLookupTable.GetFunctionPointer("ECS_DestroyEntity");
        
        SetParentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, void>)
            NativeLookupTable.GetFunctionPointer("ECS_SetParent");
        
        GetParentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_GetParent");
        
        GetEntityByNamePtr =
            (delegate* unmanaged[Cdecl]<byte*, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_GetEntityByName");
        
        RegisterComponentPtr =
            (delegate* unmanaged[Cdecl]<byte*, ulong, ulong, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_RegisterComponent");
        
        AddComponentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, void>)
            NativeLookupTable.GetFunctionPointer("ECS_AddComponent");
        
        SetComponentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, void*, void>)
            NativeLookupTable.GetFunctionPointer("ECS_SetComponent");
        
        GetComponentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, void*>)
            NativeLookupTable.GetFunctionPointer("ECS_GetComponent");
        
        HasComponentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, bool>)
            NativeLookupTable.GetFunctionPointer("ECS_HasComponent");
        
        RemoveComponentPtr =
            (delegate* unmanaged[Cdecl]<ulong, ulong, void>)
            NativeLookupTable.GetFunctionPointer("ECS_RemoveComponent");
        
        CreateSystemPtr =
            (delegate* unmanaged[Cdecl]<byte*, void*, ulong, bool, IteratorDelegate, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_CreateSystem");
    }

    internal static unsafe ulong CreateEntity(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name + '\0');

        fixed (byte* ptr = utf8)
        {
            return CreateEntityPtr(ptr);
        }
    }
    
    internal static unsafe void DestroyEntity(ulong entity)
    {
        DestroyEntityPtr(entity);
    }
    
    internal static unsafe void SetParent(ulong child, ulong parent)
    {
        SetParentPtr(child, parent);
    }
    
    internal static unsafe ulong GetParent(ulong entity)
    {
        return GetParentPtr(entity);
    }

    internal static unsafe ulong GetEntityByName(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name + '\0');

        fixed (byte* ptr = utf8)
        {
            return GetEntityByNamePtr(ptr);
        }
    }
    
    internal static unsafe ulong RegisterComponent<T>() where T: unmanaged, IComponent
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(typeof(T).AssemblyQualifiedName + '\0');

        var size = Unsafe.SizeOf<T>();
        var alignment = AlignOf<T>();
        
        fixed (byte* ptr = utf8)
        {
            var cachedId = RegisterComponentPtr(ptr, (ulong)size, (ulong)alignment);
            RegisteredComponents.Add(typeof(T), cachedId);
            
            return cachedId;
        }
    }
    
    internal static unsafe void AddComponent<T>(ulong entity, T component) where T : unmanaged, IComponent
    {
        AddComponentPtr(entity, ComponentId<T>.Value);
    }
    
    internal static unsafe void SetComponent<T>(ulong entity, T component) where T : unmanaged, IComponent
    {
        SetComponentPtr(entity, ComponentId<T>.Value, Unsafe.AsPointer(ref component));
    }
    
    internal static unsafe T GetComponent<T>(ulong entity) where T : unmanaged, IComponent
    {
        return Unsafe.AsRef<T>(GetComponentPtr(entity, ComponentId<T>.Value));
    }
    
    internal static unsafe bool HasComponent<T>(ulong entity) where T : unmanaged, IComponent
    {
        return HasComponentPtr(entity, ComponentId<T>.Value);
    }
    
    internal static unsafe void RemoveComponent<T>(ulong entity) where T : unmanaged, IComponent
    {
        RemoveComponentPtr(entity, ComponentId<T>.Value);
    }

    internal static ulong CreateSystem(string name, void*, ulong, bool,)
    {
        
    }
    
    private static unsafe int AlignOf<T>() where T : unmanaged
    {
        return sizeof(AlignOfHelper<T>) - sizeof(T);
    }
}
