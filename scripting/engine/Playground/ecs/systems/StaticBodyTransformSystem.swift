func staticbodyTransformSystem(iter: UnsafeMutableRawPointer) {
    var staticbodies = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: StaticbodyComponent.self)
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: WorldTranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: WorldRotationComponent.self)

    for x in 0..<translations.count {
        var body = staticbodies[x]

        var pos = Physics.bodyPosition(id: staticbodies[x].handle!)
        var rot = Physics.bodyRotation(id: staticbodies[x].handle!)

        translations[x].position = pos
        rotations[x].rotation = rot
    }
}
