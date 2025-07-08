func rigidbodyTransformSystem(iter: UnsafeMutableRawPointer) {
    var rigidbodies = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: RigidbodyComponent.self)
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: WorldTranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: WorldRotationComponent.self)

    for x in 0..<translations.count {
        var body = rigidbodies[x]

        var pos = Physics.bodyPosition(id: rigidbodies[x].handle!)
        var rot = Physics.bodyRotation(id: rigidbodies[x].handle!)

        translations[x].position = pos
        rotations[x].rotation = rot
    }
}
