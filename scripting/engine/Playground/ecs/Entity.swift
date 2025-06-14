public struct Entity {
    public let id: UInt64

    public var parent: Entity? {
        let id = ECSHandler.getParent(self.id)
        guard id != 0 else {
            return nil
        }

        return Entity(id: id)
    }

    internal init(id: UInt64) {
        self.id = id
    }

    public init(_ name: String) {
        self.id = ECSHandler.createEntity(name)
    }

    public func addComponent<T>(_ component: inout T) {
        ECSHandler.addComponent(id, type: T.self)
        ECSHandler.setComponent(id, data: &component)

        #if DEBUG
        let mirror = Mirror(reflecting: component)
        guard mirror.displayStyle != .class else {
            Logger.error("Component \(T.self) is not a struct. Only value types are allowed for components")
            fatalError("Component \(T.self) is not a struct. Only value types are allowed for components")
        }
        #endif
    }

    public func getComponent<T>(_ type: T.Type) -> UnsafeMutablePointer<T> {
        ECSHandler.getComponent(id, type: T.self)
    }

    public func destroyComponent<T>(_ type: T.Type) {
        ECSHandler.destroyComponent(id, type: type)
    }

    public func addTag(tag: String) {
        ECSHandler.addTag(id, tag)
    }
}

extension Entity: Sendable, Identifiable, Hashable {
    public func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    public static func == (lhs: Entity, rhs: Entity) -> Bool {
        return lhs.id == rhs.id
    }
}
