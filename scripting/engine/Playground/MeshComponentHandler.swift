internal typealias AttachMeshComponent = @convention(c) (UInt32, UnsafeMutableRawPointer?, UInt16, UnsafeMutableRawPointer?) -> UnsafeMutableRawPointer

internal enum MeshComponentHandler {
    internal nonisolated(unsafe) static var attachPtr: AttachMeshComponent!

    internal nonisolated static func setup() {
        attachPtr = NativeLookupTable.getFunctionPointer(by: "GameObject_AddMeshComponent")
    }

    internal nonisolated static func attach(to go: UInt32, component: MeshComponent) -> UnsafeMutableRawPointer {
        attachPtr(go, component.mesh?.modelHandle ?? nil, component.mesh.meshId, component.material?.materialHandle ?? nil)
    }
}
