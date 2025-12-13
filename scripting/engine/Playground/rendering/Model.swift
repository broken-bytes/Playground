public extension Rendering {
    public class Model {
        internal private(set) var handle: UnsafeMutableRawPointer!

        internal init(named: String) {
            named.withCString { str in
                handle = AssetHandler.loadModel(named: str)
            }
        }
    }
}
