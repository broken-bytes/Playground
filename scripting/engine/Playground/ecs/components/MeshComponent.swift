public struct MeshComponent {
    public let handle: UnsafeMutableRawPointer
    public let meshId: UInt64

    public init(handle: UnsafeMutableRawPointer, meshId: UInt64) {
        self.handle = handle
        self.meshId = meshId
    }
}
