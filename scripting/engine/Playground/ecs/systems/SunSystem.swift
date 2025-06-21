func sunSystem(iter: UnsafeMutableRawPointer) {
    var sun = ECSHandler.getComponentBuffer(iter: iter, slot: 0, type: SunComponent.self)[0]

    Renderer.setSun(direction: &sun.direction, colour: &sun.colour, intensity: sun.intensity)
}
