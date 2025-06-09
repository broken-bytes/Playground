enum EventHandler {
    internal typealias NativeEventCallback = @convention(c) (UnsafeRawPointer) -> Void
    internal typealias SubscribeToEventPtr = @convention(c) (Int32, NativeEventCallback) -> Void

    private static nonisolated(unsafe) var subscribePtr: SubscribeToEventPtr!

    internal static func start() {
        subscribePtr = NativeLookupTable.shared.getFunctionPointer(by: "Events_Subscribe")

        subscribePtr(EventType.system.rawValue ) { event in
            let systemEvent = event.assumingMemoryBound(to: SystemEvent.self).pointee

            Logger.info("The event is \(systemEvent.systemType)")

            switch systemEvent.systemType {
                case .quit:
                    EngineEnvironment.isRunning = false
                default:
                    fatalError("Wrong event \(systemEvent.systemType)")
            }
        }
    }
}
