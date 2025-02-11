using System.Runtime.InteropServices;

namespace Playground;

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void LookupTableDelegate(string name, IntPtr address);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void EmptyDelegate();

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void UpdateDelegate(double deltaTime);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate uint ReadBackBufferDelegate(IntPtr buffer);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate void UpdateTransformDelegate(
    uint id, 
    float positionX, 
    float positionY, 
    float positionZ, 
    float rotationX, 
    float rotationY, 
    float rotationZ
);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate uint CreateCameraDelegate(
    float fov, 
    float aspectRatio, 
    float nearClip, 
    float farClip, 
    float[] pos, 
    float[] rot,
    uint renderTextureId
);

[UnmanagedFunctionPointer(CallingConvention.Cdecl)]
internal delegate uint UpdateCameraDelegate(
    float fov, 
    float aspectRatio, 
    float nearClip, 
    float farClip, 
    float[] pos, 
    float[] rot,
    uint renderTextureId
);

