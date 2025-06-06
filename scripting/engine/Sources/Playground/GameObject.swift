public class GameObject {
    public protocol GameObjectLifeTimeDelegate: AnyObject {
        func onCreated(gameObject: GameObject)
        func onEnabled(gameObject: GameObject)
        func onDisabled(gameObject: GameObject)
        func onDestroyed(gameObject: GameObject)
    }


    public let transform: Transform = Transform()
    internal weak var parent: GameObject? = nil
    internal let id: UInt32

    @MainActor
    private static var delegates: [GameObjectLifeTimeDelegate] = []
    
    private var components: [Component] = []

    public init() {
        id = SceneManager.createGameObject()
    }

    @MainActor
    public static func register(delegate: GameObjectLifeTimeDelegate) {
        delegates.append(delegate)
    }

    @MainActor
    public static func unregister(delegate: GameObjectLifeTimeDelegate) {
        delegates.removeAll { $0 === delegate }
    }

    internal func onDestroy() {
        SceneManager.destroyGameObject(id: id)
    }
}
