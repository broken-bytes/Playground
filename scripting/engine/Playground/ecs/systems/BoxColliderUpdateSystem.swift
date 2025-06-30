public func boxColliderUpdateSystem(iter: UnsafeMutableRawPointer) {
    var boxcolliders = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: BoxColliderComponent.self)
    let entities = ECSHandler.entitiesFor(iter: iter)

    for x in 0..<translations.count {
        if boxcolliders[x].isDirty {
            var parentId: UInt64 = 0

            do {
                parentId = ECSHandler.getParentPtr(entities[x])
            }
        }
    }
}
