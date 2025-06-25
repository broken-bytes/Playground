func hierarchySystem(iter: UnsafeMutableRawPointer) {
    let translations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)
    let rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: RotationComponent.self)
    let scales = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: ScaleComponent.self)
    let worldTranslations = ECSHandler.getComponentBuffer(iter: iter, slot: 3, type: WorldTranslationComponent.self)
    let worldRotations = ECSHandler.getComponentBuffer(iter: iter, slot: 4, type: WorldRotationComponent.self)
    let worldScales = ECSHandler.getComponentBuffer(iter: iter, slot: 5, type: WorldScaleComponent.self)
    let entities = ECSHandler.entitiesFor(iter: iter)

    for x in 0..<translations.count {
        // Check if the entity has a parent
        let parent = ECSHandler.getParent(entities[x])
        if parent != 0 {
            let parentTranslationPtr = ECSHandler.getComponent(
                parent,
                type: TranslationComponent.self
            )
            let parentRotationPtr = ECSHandler.getComponent(
                parent,
                type: RotationComponent.self
            )
            let parentScalePtr = ECSHandler.getComponent(
                parent,
                type: ScaleComponent.self
            )
            // Get the parent transform
            worldTranslations[x].position = parentTranslationPtr.pointee.position + translations[x].position
            worldRotations[x].rotation = parentRotationPtr.pointee.rotation * rotations[x].rotation
            worldScales[x].scale = parentScalePtr.pointee.scale * scales[x].scale
        } else {
            worldTranslations[x].position = translations[x].position
            worldRotations[x].rotation = rotations[x].rotation
            worldScales[x].scale = scales[x].scale
        }
    }
}
