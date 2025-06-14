func cameraSystem(iter: UnsafeMutableRawPointer) {
    let cameraBuffer = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: CameraComponent.self)

    for x in 0..<cameraBuffer.count {
        cameraBuffer[x].id = UInt8.random(in: 0..<8)
    }
}
