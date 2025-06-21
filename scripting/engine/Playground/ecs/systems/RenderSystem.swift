fileprivate var ptr = UnsafeMutablePointer<DrawCall>.allocate(capacity: 131072)
fileprivate var matrices = UnsafeMutablePointer<Mat4>.allocate(capacity: 131072)

func renderSystem(iter: UnsafeMutableRawPointer) {
    let worldTranslations = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: TranslationComponent.self)
    let worldRotations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: RotationComponent.self)
    let worldScales = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: ScaleComponent.self)    
    let meshComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 3, type: MeshComponent.self)
    let materialComponents = ECSHandler.getComponentBuffer(iter: iter, slot: 4, type: MaterialComponent.self)

    let offset = ECSHandler.iteratorOffset(iter: iter)

    var buffer = UnsafeMutableBufferPointer(start: matrices.advanced(by: Int(offset)), count: worldTranslations.count)

    NativeMath.mat4FromPRSBulk(worldTranslations.baseAddress!, worldRotations.baseAddress!, worldScales.baseAddress!, buffer)

    let drawPtr = ptr.advanced(by: Int(offset))

    for x in 0..<worldTranslations.count {
        drawPtr[x] = DrawCall(
            modelHandle: meshComponents[x].handle,
            meshId: meshComponents[x].meshId,
            materialHandle: materialComponents[x].handle,
            transform: matrices[x]
        )
    }

    Renderer.batch(ptr, count: UInt16(worldTranslations.count))
}
