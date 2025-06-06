
public enum Renderer {
    internal typealias RendererStart = @convention(c) (UnsafeRawPointer, UInt32, UInt32, Bool) -> Void
    internal typealias RendererPreFraame = @convention(c) () -> Void
    internal typealias RendererUpdate = @convention(c) (Float) -> Void
    internal typealias RendererPostFraame = @convention(c) () -> Void
    internal typealias CreateCamera = @convention(c) (Float, Float, Float, Float, UnsafePointer<Float>, UnsafePointer<Float>, UInt32) -> UInt32
    internal typealias SetCameraFOV = @convention(c) (UInt32, Float) -> Void
    internal typealias SetCameraAspectRatio = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias SetCameraNear = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias SetCameraFar = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias SetCameraPosition = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias SetCameraRotation = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias SetCameraRenderTarget = @convention(c) (UnsafePointer<CChar>) -> Void
    internal typealias DestroyCamera = @convention(c) (UInt32) -> Void

    private static nonisolated(unsafe) var startPtr: RendererStart!
    private static nonisolated(unsafe) var preFramePtr: RendererPreFraame!
    private static nonisolated(unsafe) var updatePtr: RendererUpdate!
    private static nonisolated(unsafe) var postFramePtr: RendererPostFraame!
    private static nonisolated(unsafe) var createCameraPtr: CreateCamera!
    private static nonisolated(unsafe) var setCameraFOVPtr: SetCameraFOV!
    private static nonisolated(unsafe) var setCameraAspectRatio: SetCameraAspectRatio!
    private static nonisolated(unsafe) var setCameraNear: SetCameraNear!
    private static nonisolated(unsafe) var setCameraFar: SetCameraFar!
    private static nonisolated(unsafe) var setCmaeraPosition: SetCameraPosition!
    private static nonisolated(unsafe) var setCameraRotation: SetCameraRotation!
    private static nonisolated(unsafe) var setCameraRenderTarget: SetCameraRenderTarget!
    private static nonisolated(unsafe) var destroyCamera: DestroyCamera!

    internal static func start(window: UnsafeRawPointer, width: UInt32, height: UInt32, offscreen: Bool) {
        startPtr = NativeLookupTable.shared.getFunctionPointer(by: "Rendering_Init")
        preFramePtr = NativeLookupTable.shared.getFunctionPointer(by: "Rendering_PreFrame")
        updatePtr = NativeLookupTable.shared.getFunctionPointer(by: "Rendering_Update")
        postFramePtr = NativeLookupTable.shared.getFunctionPointer(by: "Rendering_PostFrame")

        startPtr(window, width, height, offscreen)
    }

    internal static func preFrame() {
        preFramePtr()
    }

    internal static func update() {
        updatePtr(Float(Time.deltaTime))
    }

    internal static func postFrame() {
        postFramePtr()
    }
}
