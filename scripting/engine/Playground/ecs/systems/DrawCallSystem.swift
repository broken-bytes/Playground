import Foundation

func drawCallSystem(iter: UnsafeMutableRawPointer) {
    let worldTransforms = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: WorldTransformComponent.self)
    let meshComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: MeshComponent.self)
    let materialComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: MaterialComponent.self)

    for x in 0..<worldTransforms.count {
        var component = DrawCallComponent(
            modelHandle: meshComponents[x].handle,
            materialHandle: materialComponents[x].handle,
            meshId: meshComponents[x].meshId,
            position: worldTransforms[x].position,
            rotation: worldTransforms[x].rotation,
            scale: worldTransforms[x].scale
        )

        let name = "DrawCall_\(UUID().uuidString)"

        let entityId = ECSHandler.createEntity(name)

        ECSHandler.addComponent(entityId, type: DrawCallComponent.self)
        ECSHandler.setComponent(entityId, data: &component)
        ECSHandler.addTag(entityId, "___internal___DrawCall")
    }
}
