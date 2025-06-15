internal typealias LookupTableDelegate = @convention(c) (UnsafePointer<CChar>, UnsafeRawPointer) -> Void
internal typealias UpdateDelegate = @convention(c) () -> Void
internal typealias OnStartedDelegate = @convention(c) () -> Void

internal struct PlaygroundCoreConfig {
    let delegate: LookupTableDelegate
    let onStarted: OnStartedDelegate
}
