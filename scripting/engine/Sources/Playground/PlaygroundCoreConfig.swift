internal typealias LookupTableDelegate = @convention(c) (UnsafePointer<CChar>, UnsafeRawPointer) -> Void

internal struct PlaygroundCoreConfig {
    let window: UnsafeRawPointer
    let delegate: LookupTableDelegate
    let width: UInt32
    let height: UInt32
    let isOffscreen: Bool
}
