using PlaygroundAssembly.ECS.Core;

namespace PlaygroundAssembly.ECS.Rendering;

[EcsComponent]
public struct CameraComponent(
    byte order,
    float fov,
    float nearPlane,
    float farPlane)
{
    public byte Order = order;
    public float Fov = fov;
    public float NearPlane = nearPlane;
    public float FarPlane = farPlane;
}
