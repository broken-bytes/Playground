func cameraSystem(iter: UnsafeMutableRawPointer) {
    var cameras = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: CameraComponent.self)
    var translations = ECSHandler.getComponentBuffer(iter: iter, slot: 1, type: WorldTranslationComponent.self)
    var rotations = ECSHandler.getComponentBuffer(iter: iter, slot: 2, type: WorldRotationComponent.self)

    for x in 0..<cameras.count {
        var cam = cameras[x]

        Renderer.addCamera(order: cam.order, fov: cam.fov, nearPlane: cam.nearPlane, farPlane: cam.farPlane, position: &translations[x].position, rotation: &rotations[x].rotation)
    }
}
