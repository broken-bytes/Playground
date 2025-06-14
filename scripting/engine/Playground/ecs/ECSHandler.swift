internal enum ECSHandler {
    internal typealias IteratorDelegate = @convention(c) (UnsafeMutableRawPointer) -> Void
    internal typealias CreateEntity = @convention(c) (UnsafeMutablePointer<CChar>) -> Void
    internal typealias DestroyEntity = @convention(c) (UInt64) -> Void
    internal typealias SetParent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias GetParent = @convention(c) (UInt64) -> UInt64
    internal typealias GetEntityByName = @convention(c) (UnsafeMutablePointer<CChar>) -> UInt64
    internal typealias RegisterComponent = @convention(c) (UnsafeMutablePointer<CChar>, UInt64, UInt64) -> UInt64
    internal typealias AddComponent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias SetComponent = @convention(c) (UInt64, UInt64, UnsafeMutableRawPointer) -> Void
    internal typealias GetComponent = @convention(c) (UInt64, UInt64) -> UnsafeRawPointer
    internal typealias HasComponent = @convention(c) (UInt64, UInt64) -> Bool
    internal typealias DestroyComponent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias CreateSystem = @convention(c) (UnsafeMutablePointer<CChar>, UnsafeMutablePointer<UInt64>, UInt64, Bool, IteratorDelegate) -> UInt64
    internal typealias GetComponentBuffer = @convention(c) (UnsafeMutableRawPointer, UInt32, UInt64) -> UnsafeMutableRawPointer
    internal typealias GetIteratorSize = @convention(c) (UnsafeMutableRawPointer) -> UInt64
    internal typealias GetEntitiesFromIterator = @convention(c) (UnsafeMutableRawPointer) -> UnsafeMutablePointer<UInt64>
    internal typealias CreateHook = @convention(c) (UInt64, IteratorDelegate, IteratorDelegate) -> Void
    internal typealias DeleteAllEntitiesByTag = @convention(c) (UInt64) -> Void
    internal typealias CreateTag = @convention(c) (UnsafeMutablePointer<CChar>) -> UInt64
    internal typealias AddTag = @convention(c) (UInt64, UInt64) -> Void

    private static nonisolated(unsafe) var createEntityPtr: CreateEntity!
    private static nonisolated(unsafe) var destroyEntityPtr: DestroyEntity!
    private static nonisolated(unsafe) var setParentPtr: SetParent!
    private static nonisolated(unsafe) var getParentPtr: GetParent!
    private static nonisolated(unsafe) var getEntityByNamePtr: GetEntityByName!
    private static nonisolated(unsafe) var registerComponentPtr: RegisterComponent!
    private static nonisolated(unsafe) var addComponentPtr: AddComponent!
    private static nonisolated(unsafe) var setComponentPtr: SetComponent!
    private static nonisolated(unsafe) var getComponentPtr: GetComponent!
    private static nonisolated(unsafe) var hasComponentPtr: HasComponent!
    private static nonisolated(unsafe) var destroyComponentPtr: DestroyComponent!
    private static nonisolated(unsafe) var createSystemPtr: CreateSystem!
    private static nonisolated(unsafe) var getComponentBufferPtr: GetComponentBuffer!
    private static nonisolated(unsafe) var getIteratorSizePtr: GetIteratorSize!
    private static nonisolated(unsafe) var getEntitiesFromIteratorPtr: GetEntitiesFromIterator!
    private static nonisolated(unsafe) var createHookPtr: CreateHook!
    private static nonisolated(unsafe) var deleteAllEntitiesByTagPtr: DeleteAllEntitiesByTag!
    private static nonisolated(unsafe) var createTagPtr: CreateTag!
    private static nonisolated(unsafe) var addTagPtr: AddTag!

    internal static func start() {
        createEntityPtr = NativeLookupTable.getFunctionPointer(by: "ECS_CreateEntity")
        destroyEntityPtr = NativeLookupTable.getFunctionPointer(by: "ECS_DestroyEntity")
        setParentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_SetParent")
        getParentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetParent")
        getEntityByNamePtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetEntityByName")
        registerComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_RegisterComponent")
        addComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_AddComponent")
        setComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_SetComponent")
        getComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetComponent")
        hasComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_HasComponent")
        destroyComponentPtr = NativeLookupTable.getFunctionPointer(by: "ECS_DestroyComponent")
        createSystemPtr = NativeLookupTable.getFunctionPointer(by: "ECS_CreateSystem")
        getComponentBufferPtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetComponentBuffer")
        getIteratorSizePtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetIteratorSize")
        getEntitiesFromIteratorPtr = NativeLookupTable.getFunctionPointer(by: "ECS_GetEntitiesFromIterator")
        createHookPtr = NativeLookupTable.getFunctionPointer(by: "ECS_CreateHook")
        deleteAllEntitiesByTagPtr = NativeLookupTable.getFunctionPointer(by: "ECS_DeleteAllEntitiesByTag")
        createTagPtr = NativeLookupTable.getFunctionPointer(by: "ECS_CreateTag")
        addTagPtr = NativeLookupTable.getFunctionPointer(by: "ECS_AddTag")
    }

    internal static func createEntity(_ name: String) -> Entity {
        
    }
}
