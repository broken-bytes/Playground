public class Camera: Component, Component.OnEnable, Component.OnDisable {
    internal let id: UInt32 = UInt32.random(in: 0...3)
    

    public func onEnable() {

    }

    public func onDisable() {

    }
}
