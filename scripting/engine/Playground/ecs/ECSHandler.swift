internal enum ECSHandler {
    internal typealias IteratorDelegate = @convention(c) (UnsafeMutableRawPointer) -> Void
    internal typealias CreateEntity = @convention(c) (UnsafePointer<CChar>) -> UInt64
    internal typealias DestroyEntity = @convention(c) (UInt64) -> Void
    internal typealias SetParent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias GetParent = @convention(c) (UInt64) -> UInt64
    internal typealias GetEntityByName = @convention(c) (UnsafePointer<CChar>) -> UInt64
    internal typealias RegisterComponent = @convention(c) (UnsafePointer<CChar>, UInt64, UInt64) -> UInt64
    internal typealias AddComponent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias SetComponent = @convention(c) (UInt64, UInt64, UnsafeMutableRawPointer) -> Void
    internal typealias GetComponent = @convention(c) (UInt64, UInt64) -> UnsafeMutableRawPointer
    internal typealias HasComponent = @convention(c) (UInt64, UInt64) -> Bool
    internal typealias DestroyComponent = @convention(c) (UInt64, UInt64) -> Void
    internal typealias CreateSystem = @convention(c) (UnsafePointer<CChar>, UnsafePointer<UInt64>, UInt64, Bool, IteratorDelegate) -> UInt64
    internal typealias GetComponentBuffer = @convention(c) (UnsafeMutableRawPointer, UInt32, UInt64, UnsafeMutablePointer<UInt64>) -> UnsafeMutableRawPointer
    internal typealias GetIteratorSize = @convention(c) (UnsafeMutableRawPointer) -> UInt64
    internal typealias GetEntitiesFromIterator = @convention(c) (UnsafeMutableRawPointer, UnsafeMutablePointer<UInt64>) -> UnsafeMutablePointer<UInt64>
    internal typealias CreateHook = @convention(c) (UInt64, IteratorDelegate, IteratorDelegate) -> Void
    internal typealias DeleteAllEntitiesByTag = @convention(c) (UInt64) -> Void
    internal typealias CreateTag = @convention(c) (UnsafePointer<CChar>) -> UInt64
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

    private static nonisolated(unsafe) var componentMapping: [ObjectIdentifier: UInt64] = [:]
    private static nonisolated(unsafe) var tagMapping: [String: UInt64] = [:]

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

    internal static nonisolated func createEntity(_ name: String) -> UInt64 {
        return name.withCString { createEntityPtr($0) }
    }

    internal static nonisolated func destroyEntity(_ id: UInt64) {
        destroyEntityPtr(id)
    }

    internal static nonisolated func setParent(_ id: UInt64, parent parentId: UInt64) {
        setParentPtr(id, parentId)
    }

    internal static nonisolated func getParent(_ id: UInt64) -> UInt64 {
        getParentPtr(id)
    }

    internal static nonisolated func getEntityByName(_ name: String) -> UInt64 {
        return name.withCString { return getEntityByNamePtr($0) }
    }

    internal static nonisolated func registerComponent<T>(_ type: T.Type) -> UInt64 {
        "\(T.self)".withCString {
            let objectId = ObjectIdentifier(T.self)
            let id = registerComponentPtr($0, UInt64(MemoryLayout<T>.size), UInt64(MemoryLayout<T>.alignment))

            componentMapping[objectId] = id

            return id
        }
    }

    internal static nonisolated func addComponent<T>(_ id: UInt64, type: T.Type) {
        let componentId = componentMapping[ObjectIdentifier(T.self)] ?? registerComponent(T.self)
        addComponentPtr(id, componentId)
    }

    internal static nonisolated func setComponent<T>(_ id: UInt64, data: inout T) {
        let componentId = componentMapping[ObjectIdentifier(T.self)] ?? registerComponent(T.self)
        setComponentPtr(id, componentId, &data)
    }

    internal static nonisolated func getComponent<T>(_ id: UInt64, type: T.Type) -> UnsafeMutablePointer<T> {
        let componentId = componentMapping[ObjectIdentifier(T.self)] ?? registerComponent(T.self)

        return getComponentPtr(id, componentId).assumingMemoryBound(to: T.self)
    }

    internal static nonisolated func hasComponent<T>(_ id: UInt64, type: T.Type) -> Bool {
        let componentId = componentMapping[ObjectIdentifier(T.self)] ?? registerComponent(T.self)

        return hasComponentPtr(id, componentId)
    }

    internal static nonisolated func destroyComponent<T>(_ id: UInt64, type: T.Type) {
        let componentId = componentMapping[ObjectIdentifier(T.self)] ?? registerComponent(T.self)

        destroyComponentPtr(id, componentId)
    }

    internal static nonisolated func createSystem(_ name: String, filter: [UInt64], multiThreaded: Bool, delegate: IteratorDelegate) -> UInt64 {
        return name.withCString { createSystemPtr($0, filter, UInt64(filter.count), multiThreaded, delegate)}
    }

    @inline(__always)
    internal static nonisolated func getComponentBuffer<T>(iter: UnsafeMutableRawPointer, slot: UInt32, type: T.Type) -> UnsafeMutableBufferPointer<T> {
        var count: UInt64 = 0
        let ptr = getComponentBufferPtr(iter, slot, UInt64(MemoryLayout<T>.size), &count)

        return UnsafeMutableBufferPointer(start: ptr.assumingMemoryBound(to: T.self), count: Int(count))
    }

    internal static nonisolated func iteratorCount(iter: UnsafeMutableRawPointer) -> UInt64 {
        return getIteratorSizePtr(iter)
    }

    internal static nonisolated func entitiesFor(iter: UnsafeMutableRawPointer) -> UnsafeBufferPointer<UInt64> {
        var count: UInt64 = 0
        return UnsafeBufferPointer(start: getEntitiesFromIteratorPtr(iter, &count), count: Int(count))
    }

    internal static nonisolated func addHook(_ id: UInt64, onAdd: IteratorDelegate, onRemove: IteratorDelegate) {
        createHookPtr(id, onAdd, onRemove)
    }

    internal static nonisolated func deleteEntities(with tag: String) {
        if let tagId = tagMapping[tag] {
            deleteAllEntitiesByTagPtr(tagId)
        }
    }

    internal static nonisolated func createTag(_ name: String) {
        tagMapping[name] = createTagPtr(name)
    }

    internal static func addTag(_ id: UInt64, _ tag: String) {
        if let tagId = tagMapping[tag] {
            addTagPtr(id, tagId)
        }
    }
}
