public class MeshComponent: Component, Component.OnStart {
    internal var native: UnsafeMutableRawPointer! = nil
    internal var mesh: Mesh! = nil
    internal var material: Material! = nil

    public func onStart() {
        Logger.info("Attaching MeshComponent to \(self.gameObject.handle.pointee)")
        native = MeshComponentHandler.attach(to: self.gameObject.handle.pointee.id, component: self)
    }
}
