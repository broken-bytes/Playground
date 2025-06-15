internal enum Profiler {
    internal typealias Profile = @convention(c) (@convention(c) () -> Void) -> Void

    internal static nonisolated(unsafe) var profilePtr: Profile!

    internal static nonisolated func start() {
    }
}
