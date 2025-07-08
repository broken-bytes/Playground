public enum Physics {
    internal typealias CreateRigidBody = @convention(c) (Float, Float, UnsafeRawPointer, UnsafeRawPointer) -> UInt64
    internal typealias CreateStaticBody = @convention(c) (UnsafeRawPointer, UnsafeRawPointer) -> UInt64
    internal typealias CreateBoxCollider = @convention(c) (UnsafeMutableRawPointer, UnsafeRawPointer, UnsafeRawPointer, UnsafeRawPointer) -> UInt64

    internal typealias AttachCollider = @convention(c) (UInt64, UInt64) -> Void

    internal typealias RemoveBody = @convention(c) (UInt64) -> Void
    internal typealias RemoveCollider = @convention(c) (UInt64) -> Void

    internal typealias GetBodyPosition = @convention(c) (UInt64, UnsafeRawPointer) -> Void
    internal typealias GetBodyRotation = @convention(c) (UInt64, UnsafeRawPointer) -> Void

    private static nonisolated(unsafe) var createRigidbodyPtr: CreateRigidBody!
    private static nonisolated(unsafe) var createStaticbodyPtr: CreateStaticBody!
    private static nonisolated(unsafe) var createBoxColliderPtr: CreateBoxCollider!

    private static nonisolated(unsafe) var attachColliderPtr: AttachCollider!

    private static nonisolated(unsafe) var removeBodyPtr: RemoveBody!
    private static nonisolated(unsafe) var removeColliderPtr: RemoveCollider!

    private static nonisolated(unsafe) var getBodyPositionPtr: GetBodyPosition!
    private static nonisolated(unsafe) var getBodyRotationPtr: GetBodyRotation!

    internal static nonisolated func setup() {
        createRigidbodyPtr = NativeLookupTable.getFunctionPointer(by: "Physics_CreateRigidBody")
        createStaticbodyPtr = NativeLookupTable.getFunctionPointer(by: "Physics_CreateStaticBody")
        createBoxColliderPtr = NativeLookupTable.getFunctionPointer(by: "Physics_CreateBoxCollider")

        attachColliderPtr = NativeLookupTable.getFunctionPointer(by: "Physics_AttachCollider")

        removeBodyPtr = NativeLookupTable.getFunctionPointer(by: "Physics_DestroyBody")
        removeColliderPtr = NativeLookupTable.getFunctionPointer(by: "Physics_DestroyCollider")

        getBodyPositionPtr = NativeLookupTable.getFunctionPointer(by: "Physics_GetBodyPosition")
        getBodyRotationPtr = NativeLookupTable.getFunctionPointer(by: "Physics_GetBodyRotation")
    }

    internal static nonisolated func createRigidBody(mass: Float, damping: Float, pos: inout Vector3, rot: inout Quaternion) -> UInt64 {
        createRigidbodyPtr(mass, damping, &pos, &rot)
    }

    internal static nonisolated func createStaticBody(pos: inout Vector3, rot: inout Quaternion) -> UInt64 {
        createStaticbodyPtr(&pos, &rot)
    }

    internal static nonisolated func createBoxCollider(material: UnsafeMutableRawPointer, offset: inout Vector3, rotation: inout Quaternion, dimensions: inout Vector3) -> UInt64 {
        createBoxColliderPtr(material, &rotation, &dimensions, &offset)
    }

    internal static nonisolated func attachCollider(body: UInt64, collider: UInt64) {
        attachColliderPtr(body, collider)
    }

    internal static nonisolated func destroyBody(body: UInt64) {
        removeBodyPtr(body)
    }

    internal static nonisolated func destroyCollider(collider: UInt64) {
        removeColliderPtr(collider)
    }

    internal static nonisolated func bodyPosition(id: UInt64) -> Vector3 {
        var vec3 = Vector3.zero
        getBodyPositionPtr(id, &vec3)

        return vec3
    }

    internal static nonisolated func bodyRotation(id: UInt64) -> Quaternion {
        var quat = Quaternion.identity
        getBodyRotationPtr(id, &quat)

        return quat
    }
}
