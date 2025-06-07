public enum SceneManager {
    internal typealias CreateGameObject = @convention(c) () -> UInt32
    internal typealias GetGameObjectTransform = @convention(c) (UInt32) -> UnsafeMutableRawPointer
    internal typealias DestroyGameObject = @convention(c) (UInt32) -> Void

    internal static nonisolated(unsafe) var gameObjects: [GameObject] = []

    private static nonisolated(unsafe) var createGameObjectPtr: CreateGameObject!
    private static nonisolated(unsafe) var getTransformPtr: GetGameObjectTransform!
    private static nonisolated(unsafe) var destroyGameObjectPtr: DestroyGameObject!

    internal static func start() {
        createGameObjectPtr = NativeLookupTable.shared.getFunctionPointer(by: "Playground_CreateGameObject")
        getTransformPtr = NativeLookupTable.shared.getFunctionPointer(by: "Playground_GetGameObjectTransform")
        destroyGameObjectPtr = NativeLookupTable.shared.getFunctionPointer(by: "Playground_DestroyGameObject")
    }

    internal static func update() {
        print("Objects in scene: \(gameObjects.count)")
        // - Run Logic

        // - Post Logic
        var destroyed = Set<UInt32>()
        let toDestroy = gameObjects.filter { $0.isDestroyed && $0.isDirty }
        for obj in toDestroy {
            destroyGameObjectPtr(obj.id)
            obj.onDestroy()
            destroyed.insert(obj.id)
            obj.isDirty = false
        }

        gameObjects.removeAll { destroyed.contains($0.id)}
    }

    internal static func createGameObject() -> GameObject {
        let id = createGameObjectPtr()
        let obj = GameObject(id: id)
        SceneManager.gameObjects.append(obj)

        return obj
    }

    internal static func getTransform(for id: UInt32) -> UnsafeMutablePointer<NativeTransform> {
        getTransformPtr(id).assumingMemoryBound(to: NativeTransform.self)
    }

    internal static func destroyGameObject(id: UInt32) {
        destroyGameObjectPtr(id)
    }
}
