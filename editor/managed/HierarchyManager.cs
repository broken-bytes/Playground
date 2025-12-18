using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace PlaygroundEditor;

public static class HierarchyManager {
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EntityCreateHook(UInt64 id, IntPtr name);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SetEntityCreateHook(IntPtr hook);
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EntityDestroyHook(UInt64 id);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SetEntityDestroyHook(IntPtr hook);
    
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void EntitySetParentHook(UInt64 id, UInt64 parentId);
    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void SetEntitySetParentHook(IntPtr hook);
    
    public delegate void OnAddEntityDelegate(Entity entity);
    public delegate void OnRemoveEntityDelegate(Entity entity);
    public delegate void OnSetParentDelegate(Entity entity, Entity parent);
    
    public static event OnAddEntityDelegate OnAddEntity;
    public static event OnRemoveEntityDelegate OnRemoveEntity;
    public static event OnSetParentDelegate OnSetParent;
    
    public static List<Entity> Entities { get; private set; } = new();
    
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void OnCreateEntity(UInt64 id, IntPtr name) {
        var entityName = Marshal.PtrToStringAnsi(name) ?? "ERROR_READING_ENTITY_NAME";
        var entity = new Entity(id, entityName, null);
        Entities.Add(entity);
        
        OnAddEntity?.Invoke(entity);
    }
    
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void OnDestroyEntity(UInt64 id) {
        var entity = Entities.FirstOrDefault(e => e.Id == id);
        if (entity != null) {
            Entities.Remove(entity);
            OnRemoveEntity?.Invoke(entity);
        }
        else {
            // Inconsistent engine <-> Editor state
            throw new Exception("Entity not found");
        }
    }
    
    [UnmanagedCallersOnly(CallConvs = new[] { typeof(CallConvCdecl) })]
    public static void OnSetEntityParent(UInt64 id, UInt64 parentId) {
        var parent  = Entities.Find((item) => item.Id == parentId);
        var child = Entities.Find((item) => item.Id == parentId);

        if (child != null && parent != null) {
            child.Parent = parent;
            OnSetParent?.Invoke(parent, child);
        }
        else {
            throw new Exception("Parent or child not found");
        }
    }
    
    public static unsafe void Setup() {
        SetEntityCreateHook createHook = Marshal.GetDelegateForFunctionPointer<SetEntityCreateHook>(NativeLookupTable.Lookup("ECS_CreateEntityHook"));
        SetEntityDestroyHook destroyHook = Marshal.GetDelegateForFunctionPointer<SetEntityDestroyHook>(NativeLookupTable.Lookup("ECS_DestroyEntityHook"));
        SetEntitySetParentHook setParentHook = Marshal.GetDelegateForFunctionPointer<SetEntitySetParentHook>(NativeLookupTable.Lookup("ECS_SetEntityParentHook"));
        
        var createPtr = (IntPtr)(delegate* unmanaged[Cdecl]<UInt64, IntPtr, void>)&OnCreateEntity;
        var destroyPtr = (IntPtr)(delegate* unmanaged[Cdecl]<UInt64, void>)&OnDestroyEntity;
        var setParentPtr = (IntPtr)(delegate* unmanaged[Cdecl]<UInt64, UInt64, void>)&OnSetEntityParent;
        
        createHook(createPtr);
        destroyHook(destroyPtr);
        setParentHook(setParentPtr);
    }
}
