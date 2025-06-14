public struct MaterialComponent {
    public let handle: UnsafeMutableRawPointer

    public init(handle: UnsafeMutableRawPointer) {
        self.handle = handle
    }
}
