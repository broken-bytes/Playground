func hierarchySystem(iter: UnsafeMutableRawPointer) {
    let transforms = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TransformComponent.self)
    let worldTransforms = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: WorldTransformComponent.self)
    let entities = ECSHandler.entitiesFor(iter: iter)
    for x in 0..<transforms.count {
        // Check if the entity has a parent
        let parent = ECSHandler.getParent(entities[x])
        if parent != 0 {
            let parentTransformPtr = ECSHandler.getComponent(
                parent,
                type: TransformComponent.self
            )
            let parentTransform = parentTransformPtr.pointee
            var transform = transforms[x]
            // Get the parent transform
            worldTransforms[x].position = parentTransform.position + transform.position
            worldTransforms[x].rotation = parentTransform.rotation * transform.rotation
            worldTransforms[x].scale = parentTransform.scale * transform.scale
        } else {
            var transform = transforms[x]
            worldTransforms[x].position = transform.position
            worldTransforms[x].rotation = transform.rotation
            worldTransforms[x].scale = transform.scale
        }
    }
}
