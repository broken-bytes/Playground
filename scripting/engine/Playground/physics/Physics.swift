public enum Physics {
    internal typealias AddBody = @convention(c) (UInt64, Float, Float, UnsafePointer<Float>, UnsafePointer<Float>) -> UInt32
    internal typealias AddBoxCollider = @convention(c) (UInt64, UnsafePointer<Float>, UnsafePointer<Float>, UnsafePointer<Float>, UnsafePointer<Float>) -> Void

    private static nonisolated(unsafe) var addBodyPtr: AddBody!
    private static nonisolated(unsafe) var addBoxColliderPtr: AddBoxCollider!

    internal static nonisolated func setup() {
        addBodyPtr = NativeLookupTable.getFunctionPointer(by: "Physics_AddBody")
        addBoxCollider = NativeLookupTable.getFunctionPointer(by: "Physics_AddBoxCollider")
    }

    internal static nonisolated func addBody(entityId: UInt64, mass: Float, damping: Float, pos: inout Vector3, rot: inout rot: Quaternion) {
        addBodyPtr(entityId, mass, damping, &pos, &rot)
    }
}
