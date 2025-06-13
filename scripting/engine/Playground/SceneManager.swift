public enum SceneManager {
    internal typealias CreateGameObject = @convention(c) () -> UnsafeMutableRawPointer
    internal typealias GetGameObjectTransform = @convention(c) (UInt32) -> UnsafeMutableRawPointer
    internal typealias DestroyGameObject = @convention(c) (UInt32) -> Void

    internal static nonisolated(unsafe) var gameObjects: [GameObject] = []

    private static nonisolated(unsafe) var createGameObjectPtr: CreateGameObject!
    private static nonisolated(unsafe) var getTransformPtr: GetGameObjectTransform!
    private static nonisolated(unsafe) var destroyGameObjectPtr: DestroyGameObject!

    internal static func start() {
        createGameObjectPtr = NativeLookupTable.getFunctionPointer(by: "GameObject_CreateGameObject")
        getTransformPtr = NativeLookupTable.getFunctionPointer(by: "GameObject_GetGameObjectTransform")
        destroyGameObjectPtr = NativeLookupTable.getFunctionPointer(by: "GameObject_DestroyGameObject")
    }

    internal static func update() {
        // - Call OnStart on all components
        for go in gameObjects {
            Logger.info("Gathering Startables")
            let startables = go.components.filter { $0 is Component.OnStart && $0.isStarted == false }
            Logger.info("Found: \(startables)")
            for startable in startables {
                Logger.info("Starting: \(startable)")
                startable.isStarted = true
                (startable as! Component.OnStart).onStart()
            }
        }

        // - Run Logic
        // - Post Logic
        var destroyed = Set<UInt32>()
        let toDestroy = gameObjects.filter { $0.isDestroyed && $0.isDirty }
        for obj in toDestroy {
            destroyGameObjectPtr(obj.handle.pointee.id)
            obj.onDestroy()
            destroyed.insert(obj.handle.pointee.id)
            obj.isDirty = false
        }

        gameObjects.removeAll { destroyed.contains($0.handle.pointee.id)}
    }

    internal static func createGameObject() -> GameObject {
        let handle = createGameObjectPtr().assumingMemoryBound(to: GameObject.NativeGameObjectHandle.self)
        let obj = GameObject(handle: handle)
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
