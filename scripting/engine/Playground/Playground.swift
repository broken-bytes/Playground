internal typealias AssemblyRegisterCallback = @convention(c) (UnsafePointer<CChar>) -> UInt32

@_cdecl("PlaygroundMain")
public func playgroundMain(window: UnsafeRawPointer, width: UInt32, height: UInt32, isDebug: Bool) {
    let engine = Engine(window: window, width: width, height: height, debug: isDebug)
    engine.start()
}
