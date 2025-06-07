import Foundation

public class GameObject: Object {
    public protocol GameObjectLifeTimeDelegate: AnyObject {
        func onCreated(gameObject: GameObject)
        func onEnabled(gameObject: GameObject)
        func onDisabled(gameObject: GameObject)
        func onDestroyed(gameObject: GameObject)
    }

    internal let _transform: Transform
    public var transform: Transform {
        checkLifetime()

        return _transform
    }

    public internal(set) var name: String = ""
    internal weak var parent: GameObject? = nil
    internal let id: UInt32
    internal var isDestroyed = false
    internal var isDirty = false

    @inline(__always)
    func checkLifetime() {
        if isDestroyed && !isDirty {
            assertionFailure("Attempted to use a destroyed GameObject!")
        }
    }

    @MainActor
    private static var delegates: [GameObjectLifeTimeDelegate] = []
    private var components: [Component] = []

    internal init(id: UInt32) {
        self.id = id
        _transform = Transform(id: id)
    }

    public func destroy() {
        self.isDestroyed = true
        self.isDirty = true
    }

    public func attach<T>(_ component: T.Type) where T: Component {
        checkLifetime()

        let component = T()

        components.append(component)
    }

    public func retrieve<T>() -> T? where T: Component {
        checkLifetime()

        return components.first { $0 is T } as? T
    }

    internal func onDestroy() {
        for comp in components {
            if let destroy = comp as? Component.OnDestroy {
                destroy.onDestroy()
            }
        }
    }
}

extension Optional where Wrapped: GameObject {
    static func ==(lhs: Wrapped?, rhs: Any?) -> Bool {
        guard let obj = lhs else { return true }
        return obj.isDestroyed
    }

    static func !=(lhs: Wrapped?, rhs: Any?) -> Bool {
        return !(lhs == rhs)
    }
}

extension GameObject: CustomDebugStringConvertible {
    public var debugDescription: String {
        if isDestroyed {
            return "null (destroyed GameObject)"
        }
        return "GameObject<\(name)>"
    }
}

extension GameObject {
    public static func ==(lhs: GameObject, rhs: GameObject?) -> Bool {
        if lhs.isDestroyed && rhs == nil {
            return true
        }

        if !lhs.isDestroyed || rhs == nil {
            return false
        }

        guard let rhs = rhs else {
            return false
        }

        let lhsDestroyed = lhs.isDestroyed
        let rhsDestroyed = rhs.isDestroyed

        if lhsDestroyed && rhsDestroyed {
            return true
        }

        if lhsDestroyed || rhsDestroyed {
            return false
        }

        return lhs === rhs
    }
}

extension GameObject {
    @MainActor
    public static func register(delegate: GameObjectLifeTimeDelegate) {
        delegates.append(delegate)
    }

    @MainActor
    public static func unregister(delegate: GameObjectLifeTimeDelegate) {
        delegates.removeAll { $0 === delegate }
    }
}
