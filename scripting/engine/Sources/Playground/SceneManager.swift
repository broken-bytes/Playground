public enum SceneManager {
    internal typealias CreateGameObject = @convention(c) () -> UInt32
    internal typealias DestroyGameObject = @convention(c) (UInt32) -> Void

    private static nonisolated(unsafe) var createGameObjectPtr: CreateGameObject!
    private static nonisolated(unsafe) var destroyGameObjectPtr: DestroyGameObject!

    internal static func start() {
        createGameObjectPtr = NativeLookupTable.shared.getFunctionPointer(by: "Playground_CreateGameObject")
        destroyGameObjectPtr = NativeLookupTable.shared.getFunctionPointer(by: "Playground_DestroyGameObject")
    }

    internal static func update() {
    }

    internal static func createGameObject() -> UInt32 {
        createGameObjectPtr()
    }

    internal static func destroyGameObject(id: UInt32) {
        destroyGameObjectPtr(id)
    }
}
