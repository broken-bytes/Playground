#if os(Windows)
import WinSDK
#endif

internal typealias CoreEngineStartup = @convention(c) (UnsafeMutableRawPointer) -> Void

internal final class Engine: @unchecked Sendable {
    private let window: UnsafeRawPointer
    private let width: UInt32
    private let height: UInt32
    private let offscreen: Bool

    private var isRunning = true
    let clock = ContinuousClock()
    private var lastTickTime: ContinuousClock.Instant

    internal init(window: UnsafeRawPointer, width: UInt32, height: UInt32, debug: Bool) {
        lastTickTime = clock.now
        self.window = window
        self.width = width
        self.height = height
        self.offscreen = false

        var config = PlaygroundCoreConfig(window: window, delegate: {
            NativeLookupTable.shared.addEntry(key: String(cString: $0), ptr: $1)
        }, width: width, height: height, isOffscreen: false)

        initEngineCore(config: &config)

        start()
    }

    func start() {
        SceneManager.start()
        Input.shared.start()

        mainLoop()
    }

    private func mainLoop() {
        while isRunning {
            let now = clock.now
            Time.deltaTime = Double(lastTickTime.duration(to: now).components.attoseconds) / 1000000000000000000.0
            lastTickTime = now

            mainTick()
        }
    }

    private func mainTick() {
        Input.shared.update()
        for x in 0...Int.random(in: 10...1000) {
            var objc = SceneManager.createGameObject()
            objc.attach(TestComponent.self)
            objc.transform.position = .one
            objc.destroy()
        }
        SceneManager.update()
    }

    private func initEngineCore(config: inout PlaygroundCoreConfig) {
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
}
