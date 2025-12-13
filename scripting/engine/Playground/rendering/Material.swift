public extension Rendering {
    public class Material {
        internal private(set) var handle: UnsafeMutableRawPointer! // The handle to the material

        internal init (named: String) {
            named.withCString { str in
                handle = AssetHandler.loadMaterial(named: str)
            }
        }

        deinit {
            // TODO: Add unloading
        }
    }
}
