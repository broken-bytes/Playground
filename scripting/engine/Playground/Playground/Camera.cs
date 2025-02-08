namespace Playground;

public partial class Camera : Component
{
    public float Fov = 90;
    public float AspectRatio = 1f;
    public float NearClip = 0.1f;
    public float FarClip = 1000.0f;
    public LayerMask LayerMask = 0;
}
