public func cameraMoveTest(iter: UnsafeMutableRawPointer) {
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)
    var cameras = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: CameraComponent.self)

    for x in 0..<translations.count {
        translations[x].position.z += Input.getAxis("horizontal")
    }
}
