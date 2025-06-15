public struct MeshComponent {
    public let handle: UnsafeMutableRawPointer
    public let meshId: UInt16

    public init(handle: UnsafeMutableRawPointer, meshId: UInt16) {
        self.handle = handle
        self.meshId = meshId
    }
}
