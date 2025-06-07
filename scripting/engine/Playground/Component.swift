public class Component {
    public enum Lifecycle {
        case onCreate
        case onStart
        case onEnable
        case onUpdate
        case onDisable
        case onDestroy
    }

    public protocol OnCreate {
        func onCreate()
    }

    public protocol OnStart {
        func onStart()
    }

    public protocol OnEnable {
        func onEnable()
    }

    public protocol OnUpdate {
        func onUpdate()
    }

    public protocol OnDisable {
        func onDisable()
    }

    public protocol OnDestroy {
        func onDestroy()
    }

    internal var isAwake: Bool = false
    internal var isStarted: Bool = false
    internal var isDestroyed: Bool = false
    public unowned(unsafe) var gameObject: GameObject?

    public required init() {}
}
