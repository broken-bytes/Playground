public struct TransformComponent {
    var position: Vector3
    var rotation: Quaternion
    var scale: Vector3

    public init(
        position: Vector3,
        rotation: Quaternion,
        scale: Vector3
    ) {
        self.position = position
        self.rotation = rotation
        self.scale = scale
    }
}

public func rotate(_ transform: inout TransformComponent, around axis: Axis, by angle: Float, in space: Space = .local) {
    let axisVector: Vector3 = {
        switch axis {
            case .x: return Vector3(x: 1, y: 0, z: 0)
            case .y: return Vector3(x: 0, y: 1, z: 0)
            case .z: return Vector3(x: 0, y: 0, z: 1)
        }
    }()

    let rotationAxis: Vector3
    switch space {
        case .local:
            rotationAxis = transform.rotation * axisVector // rotate axis by current rotation
        case .world:
            rotationAxis = axisVector
    }

    let rotationQuat = Quaternion(angle: angle, axis: rotationAxis)
    transform.rotation = rotationQuat * transform.rotation
}
