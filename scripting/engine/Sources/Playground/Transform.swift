import Foundation

public class Transform {
    public unowned var gameObject: GameObject! = nil

    public var position: Vector3 = .zero
    public var rotation: Quaternion = .identity
    public var scale: Vector3 = .one
    public var euler: Vector3 { rotation.euler }

    internal var parent: Transform? {
        gameObject.parent?.transform
    }

    internal init() {

    }

    public func translate(translation: Vector3) {
        self.position += translation
    }

    public func rotate(around: Axis, by rotation: Float, in space: Space = .local) {
        let axisVector: Vector3 = {
            switch around {
                case .x: return Vector3(x: 1, y: 0, z: 0)
                case .y: return Vector3(x: 0, y: 1, z: 0)
                case .z: return Vector3(x: 0, y: 0, z: 1)
            }
        }()

        let worldAxis = space == .local ? self.localToWorld(axisVector) : axisVector

        let rotationQuat = Quaternion(angle: rotation, axis: worldAxis)
        self.rotation = rotationQuat * self.rotation
    }

    public func localToWorld(_ point: Vector3) -> Vector3 {
        var worldPoint = point * scale
        worldPoint = rotation * worldPoint
        worldPoint += position
        if let parent = parent {
            return parent.localToWorld(worldPoint)
        } else {
            return worldPoint
        }
    }
}
