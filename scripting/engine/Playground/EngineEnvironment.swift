internal enum EngineEnvironment {
    internal nonisolated(unsafe) static var isRunning = true {
        didSet {
            Logger.info("IsRunning was set to \(isRunning)")
        }
    }
}
