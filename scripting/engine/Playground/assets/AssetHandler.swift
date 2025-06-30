internal typealias LoadModel = @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer
internal typealias LoadMaterial = @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer
internal typealias LoadPhysicsMaterial = @convention(c) (UnsafePointer<CChar>) -> UnsafeMutableRawPointer

internal enum AssetHandler {
    internal nonisolated(unsafe) static var loadModelPtr: LoadModel!
    internal nonisolated(unsafe) static var loadMaterialPtr: LoadMaterial!
    internal nonisolated(unsafe) static var loadPhysicsMaterialPtr: LoadPhysicsMaterial!

    internal nonisolated static func setup() {
        loadModelPtr = NativeLookupTable.getFunctionPointer(by: "AssetManager_LoadModel")
        loadMaterialPtr = NativeLookupTable.getFunctionPointer(by: "AssetManager_LoadMaterial")
        loadPhysicsMaterialPtr = NativeLookupTable.getFunctionPointer(by: "AssetManager_LoadPhysicsMaterial")
    }

    internal nonisolated static func loadModel(named: String) -> UnsafeMutableRawPointer {
        named.withCString {
            loadModelPtr($0)
        }
    }

    internal nonisolated static func loadMaterial(named: String) -> UnsafeMutableRawPointer {
        named.withCString {
            loadMaterialPtr($0)
        }
    }

    internal nonisolated static func loadPhysicsMaterial(named: String) -> UnsafeMutableRawPointer {
        named.withCString {
            loadPhysicsMaterialPtr($0)
        }
    }
}
