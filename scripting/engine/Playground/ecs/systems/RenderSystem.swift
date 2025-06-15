func renderSystem(iter: UnsafeMutableRawPointer) {
    let drawCallsComps = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: DrawCallComponent.self)

    var ptr = UnsafeMutablePointer<DrawCall>.allocate(capacity: drawCallsComps.count)

    for x in 0..<drawCallsComps.count {

        ptr.advanced(by: x).pointee = DrawCall(
            modelHandle: drawCallsComps[x].modelHandle,
            meshId: drawCallsComps[x].meshId,
            materialHandle: drawCallsComps[x].materialHandle,
            position: drawCallsComps[x].position,
            rotation: drawCallsComps[x].rotation,
            scale: drawCallsComps[x].scale
        )
    }

    Renderer.batch(ptr, count: UInt16(drawCallsComps.count))
    ptr.deallocate()
    ECSHandler.deleteEntities(with: "___internal___DrawCall")
}
