public class Mesh {
    internal let modelHandle: UnsafeMutableRawPointer! // The handle to the model
    internal let meshId: UInt16 // The sub index of this mesh in the model

    internal init (named: String, at meshId: UInt16) {
        self.meshId = meshId

        modelHandle = AssetHandler.loadModel(named: named)
    }

    deinit {
        // TODO: Add unloading
    }
}
