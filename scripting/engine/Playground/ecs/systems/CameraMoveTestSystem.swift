public func cameraMoveTest(iter: UnsafeMutableRawPointer) {
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: RotationComponent.self)
    var cameras = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: CameraComponent.self)

    for x in 0..<translations.count {
        rotations[x].rotation = rotations[x].rotation.rotatedAroundLocal(axis: .up, angle: Input.getAxis("lookx"))
        rotations[x].rotation = rotations[x].rotation.rotatedAroundLocal(axis: .left, angle: -Input.getAxis("looky"))

        let movement = Vector3(x: Input.getAxis("horizontal"), y: 0, z: Input.getAxis("vertical")) * 5 * Float(Time.deltaTime)
        translations[x].position = translateLocal(position: translations[x].position, rotation: rotations[x].rotation, translation: movement)
    }
}
