public func cameraMoveTest(iter: UnsafeMutableRawPointer) {
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: RotationComponent.self)

    for x in 0..<translations.count {
        let lookX = Input.getAxis("lookx")
        let lookY = Input.getAxis("looky")

        let yaw = Quaternion(angle: degToRad(lookX), axis: Vector3.up)
        let pitch = Quaternion(angle: degToRad(lookY), axis: Vector3.right)

        rotations[x].rotation = (yaw * rotations[x].rotation) * pitch

        let movement = Vector3(x: Input.getAxis("horizontal"), y: 0, z: Input.getAxis("vertical")) * 5 * Float(Time.deltaTime)
        translations[x].position = translateLocal(position: translations[x].position, rotation: rotations[x].rotation, translation: movement)
    }
}
