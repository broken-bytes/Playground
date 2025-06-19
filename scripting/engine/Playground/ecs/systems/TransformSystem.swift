func transformSystem(iter: UnsafeMutableRawPointer) {
    let translations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)

    for x in 0..<translations.count {
        translations[x].position = Vector3(x: translations[x].position.x + 1 * Float(Time.deltaTime) + 1, y: translations[x].position.y * Float(Time.deltaTime), z: translations[x].position.z + 1 * Float(Time.deltaTime))
    }
}
