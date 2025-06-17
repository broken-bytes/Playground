var ptr = UnsafeMutablePointer<DrawCall>.allocate(capacity: 131072)

func renderSystem(iter: UnsafeMutableRawPointer) {
    let worldTransforms = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: WorldTransformComponent.self)
    let meshComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: MeshComponent.self)
    let materialComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: MaterialComponent.self)

    let offset = ECSHandler.iteratorOffset(iter: iter)

    var pos = Vector3.zero
    var rot = Quaternion.identity
    var scale = Vector3.zero

    for x in 0..<worldTransforms.count {
        pos = worldTransforms[x].position
        rot = worldTransforms[x].rotation
        scale = worldTransforms[x].scale

        ptr.advanced(by: Int(offset) + x).pointee = DrawCall(
            modelHandle: meshComponents[x].handle,
            meshId: meshComponents[x].meshId,
            materialHandle: materialComponents[x].handle,
            transform: NativeMath.mat4FromPRS(&pos, &rot, &scale)
        )
    }

    Renderer.batch(ptr, count: UInt16(worldTransforms.count))
}
