#if os(Windows)
import WinSDK
#endif

@component
public struct MaterialComponent: ~Copyable {
    public var name: UnsafeMutablePointer<CChar> {
        didSet {
            handle = AssetHandler.loadModel(named: name)
        }
    }
    public var handle: UnsafeMutableRawPointer

    public init(name: String) {
        self.name = .allocate(capacity: name.count)
        self.handle = AssetHandler.loadMaterial(named: name)

        name.withCString { str in
            strncpy(self.name, str, name.count)
        }
    }

    public init(handle: UnsafeMutableRawPointer) {
        self.name = .allocate(capacity: 1)
        self.handle = handle
    }

    deinit {
        name.deallocate()
    }
}
