using System.Runtime.InteropServices;
using Playground.Internal;

namespace Playground;

public partial class Camera
{
    private static readonly CreateCameraDelegate _createCameraPtr; 
    private static readonly UpdateCameraDelegate _updateCameraPtr;
    private uint _handle;

    static Camera()
    {
        _createCameraPtr = Marshal.GetDelegateForFunctionPointer<CreateCameraDelegate>(NativePointerTable.GetPointer("Rendering_CreateCamera"));
    }

    public override void OnAwake()
    {
        _handle = _createCameraPtr(
            Fov, 
            AspectRatio, 
            NearClip, 
            FarClip,
            [GameObject.Transform.Position.X, GameObject.Transform.Position.Y, GameObject.Transform.Position.Z],
            [GameObject.Transform.Rotation.X, GameObject.Transform.Rotation.Y, GameObject.Transform.Rotation.Z, GameObject.Transform.Rotation.W],
            0);
    }

    public override void OnUpdate()
    {
 
    }
}
