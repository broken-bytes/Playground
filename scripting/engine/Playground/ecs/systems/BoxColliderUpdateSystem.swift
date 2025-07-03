public func boxColliderUpdateSystem(iter: UnsafeMutableRawPointer) {
    var boxcolliders = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: BoxColliderComponent.self)
    let entities = ECSHandler.entitiesFor(iter: iter)

    for x in 0..<boxcolliders.count {
        var collider = boxcolliders[x]
        var offset = collider.offset
        var dimensions = collider.dimensions
        var rotation = collider.rotation

        if collider.isDirty {
            if collider.handle == nil {
                collider.handle = Physics.createBoxCollider(material: collider.material, offset: &offset, rotation: &rotation, dimensions: &dimensions)
            }

            var parentId: UInt64 = 0
            var foundBody = false

            if (ECSHandler.hasComponent(entities[x], type: RigidbodyComponent.self)) {
                let comp = ECSHandler.getComponent(entities[x], type: RigidbodyComponent.self)

                if (comp.pointee.handle == nil) {
                    continue
                }

                Physics.attachCollider(body: comp.pointee.handle!, collider: collider.handle!)

                boxcolliders[x].isDirty = false

                continue
            } else {
                repeat {
                    parentId = ECSHandler.getParent(entities[x])

                    if (ECSHandler.hasComponent(parentId, type: RigidbodyComponent.self)) {
                        let comp = ECSHandler.getComponent(parentId, type: RigidbodyComponent.self)

                        if (comp.pointee.handle == nil) {
                            continue
                        }

                        Physics.attachCollider(body: comp.pointee.handle!, collider: collider.handle!)

                        boxcolliders[x].isDirty = false

                        foundBody = true
                    }
                } while parentId != 0 && foundBody == false
            }
        }
    }
}
