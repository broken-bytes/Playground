using Playground.Core.Logging;

namespace Playground.Core.Assets;

internal static class AssetApi
{
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadModelPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadMaterialPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, IntPtr> LoadPhysicsMaterialPtr;
    internal static unsafe delegate* unmanaged[Cdecl]<byte*, byte*, ulong*, void> LoadScenePtr;

    internal static void Setup()
    {
        unsafe
        {
            LoadModelPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadModelByName");

            LoadMaterialPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadMaterialByName");

            LoadPhysicsMaterialPtr =
                (delegate* unmanaged[Cdecl]<byte*, IntPtr>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadPhysicsMaterialByName");
            
            LoadScenePtr =
                (delegate* unmanaged[Cdecl]<byte*, byte*, ulong*, void>)
                NativeLookupTable.GetFunctionPointer("AssetManager_LoadSceneByName");
        }
    }

    internal static unsafe IntPtr LoadModel(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name);

        fixed (byte* ptr = utf8)
        {
            return LoadModelPtr(ptr);
        }
    }

    internal static unsafe IntPtr LoadMaterial(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name);

        fixed (byte* ptr = utf8)
        {
            return LoadMaterialPtr(ptr);
        }
    }

    internal static unsafe IntPtr LoadPhysicsMaterial(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name);

        fixed (byte* ptr = utf8)
        {
            return LoadPhysicsMaterialPtr(ptr);
        }
    }
    
    internal static unsafe string LoadScene(string name)
    {
        var utf8 = System.Text.Encoding.UTF8.GetBytes(name);

        ulong count = 0;
        
        fixed (byte* ptr = utf8)
        {
            LoadScenePtr(ptr, null, &count);
            var data = new byte[count];
            fixed (byte* dataPtr = data)
            {
                LoadScenePtr(ptr, dataPtr, &count);

                return System.Text.Encoding.UTF8.GetString(dataPtr, (int)count);
            }
        }
    }
}
