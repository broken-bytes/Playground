func rigidBodyUpdateSystem(iter: UnsafeMutableRawPointer) {
    var rigidbodies = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: RigidbodyComponent.self)
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: WorldTranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: WorldRotationComponent.self)
    let entities = ECSHandler.entitiesFor(iter: iter)

    for x in 0..<translations.count {
        var body = rigidbodies[x]
        var translation = translations[x].position
        var rotation = rotations[x].rotation

        if body.isDirty {
            if body.handle == nil {
                rigidbodies[x].handle = Physics.createRigidBody(mass: body.mass, damping: body.damping, pos: &translation, rot: &rotation)
            }

            body.isDirty = false
        }
    }
}
