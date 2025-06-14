public class Model {
    internal let handle: UnsafeMutableRawPointer!

    internal init(named: String) {
        handle = AssetHandler.loadModel(named: named)
    }
}
