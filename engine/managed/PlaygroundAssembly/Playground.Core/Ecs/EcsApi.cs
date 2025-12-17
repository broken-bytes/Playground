using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using Playground.Core.Logging;

namespace Playground.Core.Ecs;

using unsafe IteratorDelegate = delegate* unmanaged[Cdecl]<IntPtr, void>;

[StructLayout(LayoutKind.Sequential)]
struct AlignOfHelper<T> where T : unmanaged
{
    public byte dummy;
    public T data;
}

internal static class EcsApi
{
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
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, EcsFilter*, ulong, bool, IteratorDelegate, ulong> CreateSystemPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<IntPtr, ulong> GetIteratorSystemPtr;
    
    internal static readonly Dictionary<Type, ulong> RegisteredComponents = new();
    
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
            NativeLookupTable.GetFunctionPointer("ECS_DestroyComponent");
        
        CreateSystemPtr =
            (delegate* unmanaged[Cdecl]<byte*, void*, ulong, bool, IteratorDelegate, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_CreateSystem");
        
        GetIteratorSystemPtr =
            (delegate* unmanaged[Cdecl]<IntPtr, ulong>)
            NativeLookupTable.GetFunctionPointer("ECS_GetIteratorSystem");
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
    
    internal static unsafe ulong RegisterComponent<T>() where T: unmanaged
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(typeof(T).Name + '\0');

        var size = Unsafe.SizeOf<Type>();
        
        fixed (byte* ptr = utf8)
        {
            var cachedId = RegisterComponentPtr(ptr, (ulong)size, (ulong)size);
            RegisteredComponents.Add(typeof(T), cachedId);
            
            return cachedId;
        }
    }
    
    internal static unsafe ulong RegisterComponent(Type type)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(type.Name + '\0');

        var size = Marshal.SizeOf(type);
        var align = GetAlignment(type);
        
        Logger.Info("Registering component: " + type.Name + " with size: " + size + " and alignment: " + align + "");
        
        fixed (byte* ptr = utf8)
        {
            var cachedId = RegisterComponentPtr(ptr, (ulong)size, (ulong)align);
            RegisteredComponents.Add(type, cachedId);
            
            return cachedId;
        }
    }
    
    internal static unsafe void AddComponent<T>(ulong entity) where T : unmanaged
    {
        AddComponentPtr(entity, IdFor<T>());
    }
    
    internal static unsafe void SetComponent<T>(ulong entity, T component) where T : unmanaged
    {
        SetComponentPtr(entity, IdFor<T>(), Unsafe.AsPointer(ref component));
    }
    
    internal static unsafe T GetComponent<T>(ulong entity) where T : unmanaged
    {
        return Unsafe.AsRef<T>(GetComponentPtr(entity, IdFor<T>()));
    }
    
    internal static unsafe bool HasComponent<T>(ulong entity) where T : unmanaged
    {
        return HasComponentPtr(entity, IdFor<T>());
    }
    
    internal static unsafe void RemoveComponent<T>(ulong entity) where T : unmanaged
    {
        RemoveComponentPtr(entity, IdFor<T>());
    }

    internal static unsafe ulong CreateSystem(string name, EcsFilter[] filter, bool isMultithreaded, IteratorDelegate system)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name);

        fixed (byte* ptr = utf8)
        {
            if (filter.Length == 0)
            {
                return CreateSystemPtr(ptr, null, 0, isMultithreaded, system);
            }
            fixed (EcsFilter* array = &filter[0])
            {
                return CreateSystemPtr(ptr, array, (ulong)filter.Length, isMultithreaded, system);
            }
        }
    }
    
    internal static unsafe ulong GetIteratorSystem(IntPtr iterator)
    {
        return GetIteratorSystemPtr(iterator);
    }

    private static ulong IdFor<T>() where T : unmanaged
    {
        if (RegisteredComponents.TryGetValue(typeof(T), out var value))
        {
            return value;
        }
        
        throw new Exception($"Component {typeof(T).Name} is not registered.");
    }
    
    private static int GetAlignment(Type type)
    {
        if (!type.IsValueType)
            throw new InvalidOperationException("Only value types supported");

        int maxAlignment = 1;

        foreach (var field in type.GetFields(
             BindingFlags.Instance |
             BindingFlags.Public |
             BindingFlags.NonPublic))
        {
            Logger.Info("Field: " + field.Name);
            int fieldAlignment = GetFieldAlignment(field.FieldType);
            if (maxAlignment < fieldAlignment)
                maxAlignment = fieldAlignment;
        }

        return maxAlignment;
    }

    private static int GetFieldAlignment(Type fieldType)
    {
        if (fieldType.IsPointer || fieldType == typeof(IntPtr))
            return IntPtr.Size;

        if (fieldType.IsEnum)
            return GetFieldAlignment(Enum.GetUnderlyingType(fieldType));

        if (fieldType == typeof(byte) || fieldType == typeof(sbyte))
            return 1;

        if (fieldType == typeof(short) || fieldType == typeof(ushort))
            return 2;

        if (fieldType == typeof(int) || fieldType == typeof(uint) || fieldType == typeof(float))
            return 4;

        if (fieldType == typeof(long) || fieldType == typeof(ulong) || fieldType == typeof(double))
            return 8;

        if (fieldType.IsValueType)
        {
            int size = Marshal.SizeOf(fieldType);

            if (size >= 8) return 8;
            if (size >= 4) return 4;
            if (size >= 2) return 2;
            return 1;
        }

        throw new NotSupportedException($"Unsupported field type: {fieldType}");
    }
}
