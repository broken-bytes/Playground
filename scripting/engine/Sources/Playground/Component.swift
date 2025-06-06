public class Component {
    public protocol OnStart {
        func onStart()
    }

    public protocol OnUpdate {
        func onUpdate()
    }

    public protocol OnDestroy {
        func onDestroy()
    }

    public protocol OnEnable {
        func onEnable()
    }

    public protocol OnDisable {
        func onDisable()
    }

    public protocol OnCreate {
        func onCreate()
    }

    internal var isAwake: Bool = false
    internal var isStarted: Bool = false
    internal var isDestroyed: Bool = false
    public unowned var gameObject: GameObject?

    public init() {}
}
