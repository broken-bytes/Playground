#if os(Windows)
import WinSDK
#endif

internal typealias AssemblyRegisterCallback = @convention(c) (UnsafePointer<CChar>) -> UInt32
internal typealias CoreEngineStartup = @convention(c) (UnsafeMutableRawPointer) -> Void

@_cdecl("PlaygroundMain")
public func playgroundMain(window: UnsafeRawPointer, width: UInt32, height: UInt32, isDebug: Bool) {
    var config = PlaygroundCoreConfig(window: window, delegate: {
        NativeLookupTable.shared.addEntry(key: String(cString: $0), ptr: $1)
    }, width: width, height: height, isOffscreen: false, onUpdate: onUpdate)

    initEngineCore(config: &config)
    startUp()
}

func startUp() {
    Logger.start()
    SceneManager.start()
    Input.start()
    EventHandler.start()
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

func onUpdate() {
    SceneManager.update()
}
