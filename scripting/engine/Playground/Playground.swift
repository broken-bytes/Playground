#if os(Windows)
import WinSDK
#endif

internal typealias CoreEngineStartup = @convention(c) (UnsafeMutableRawPointer) -> Void

@_cdecl("PlaygroundMain")
public func playgroundMain(window: UnsafeRawPointer, width: UInt32, height: UInt32, isDebug: Bool) {
    var config = PlaygroundCoreConfig(
        window: window,
        delegate: onAddLookupEntry,
        width: width,
        height: height, 
        isOffscreen: false, 
        onUpdate: onUpdate,
        onStarted: startUp
    )

    initEngineCore(config: &config)
}

func startUp() {
    Logger.start()
    Logger.info("Playground Scripting Layer Startup")
    // System Init
    Logger.info("Started Logger")
    SceneManager.start()
    Logger.info("Started SceneManager")
    Input.start()
    Logger.info("Started Input")
    EventHandler.start()
    Logger.info("Started EventHandler")

    // Swift <-> Native Handlers
    AssetHandler.setup()
    MeshComponentHandler.setup()

    var go = SceneManager.createGameObject()
    let mesh = Mesh(named: "cube.mod", at: 0)
    let material = Material(named: "default.mat")
    var meshComponent = go.attach(MeshComponent.self)
    meshComponent.mesh = mesh
    meshComponent.material = material
}

func initEngineCore(config: inout PlaygroundCoreConfig) {
    #if os(Windows)
    guard let lib = LoadLibraryA("PlaygroundCore.dll") else {
        fatalError("Missing Engine Core")
    }

    guard let ptr = GetProcAddress(lib, "PlaygroundCoreMain") else {
        fatalError("Startup Routine missing")
    }

    let startup = unsafeBitCast(ptr, to: CoreEngineStartup.self)

    withUnsafeMutablePointer(to: &config) {
        startup(UnsafeMutableRawPointer($0))
    }

    #endif
}

func onAddLookupEntry(key: UnsafePointer<CChar>, value: UnsafeRawPointer) {
    NativeLookupTable.addEntry(key: String(cString: key), ptr: value)
}

func onUpdate() {
    SceneManager.update()
}
