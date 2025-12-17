namespace Playground.Core.Assets;

internal static class AssetApi
{
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadModelPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadMaterialPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadPhysicsMaterialPtr;

    internal static void Setup()
    {
        unsafe
        {
            LoadModelPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadModel");

            LoadMaterialPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadMaterial");

            LoadPhysicsMaterialPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadPhysicsMaterial");
        }
    }

    internal static unsafe IntPtr LoadModel(byte* name)
    {
        return LoadModelPtr(name);
    }

    internal static unsafe IntPtr LoadMaterial(byte* name)
    {
        return LoadMaterialPtr(name);
    }

    internal static unsafe IntPtr LoadPhysicsMaterial(byte* name)
    {
        return LoadPhysicsMaterialPtr(name);
    }
}
