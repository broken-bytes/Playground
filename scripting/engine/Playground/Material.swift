public class Material {
    internal let materialHandle: UnsafeMutableRawPointer! // The handle to the material

    internal init (named: String) {
        materialHandle = AssetHandler.loadMaterial(named: named)
    }

    deinit {
        // TODO: Add unloading
    }
}
