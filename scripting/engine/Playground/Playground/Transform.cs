namespace Playground;

public struct Transform {
    public Vector3 Position;
    public Vector3 LocalPosition { get; set; }
    public Quaternion Rotation { get; set; }
    public Vector3 EulerAngles;
    public Vector3 Scale;
}
