#if os(Windows)
import WinSDK
#endif

@component
public struct MeshComponent : ~Copyable {
    public var name: UnsafeMutablePointer<CChar> {
        didSet {
            handle = AssetHandler.loadModel(named: name)
        }
    }
    public var handle: UnsafeMutableRawPointer
    public var meshId: UInt16

    public init(name: String, meshId: UInt16) {
        self.name = .allocate(capacity: name.count)
        self.handle = AssetHandler.loadModel(named: name)
        self.meshId = meshId

        name.withCString { str in
            strncpy(self.name, str, name.count)
        }
    }

    public init(handle: UnsafeMutableRawPointer, meshId: UInt16) {
        self.name = .allocate(capacity: 1)
        self.handle = handle
        self.meshId = meshId
    }

    deinit {
        name.deallocate()
    }
}
