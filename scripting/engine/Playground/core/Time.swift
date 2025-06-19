public enum Time {
    public nonisolated(unsafe) static var deltaTime: Double {
        getDeltaTimePtr()
    }

    public nonisolated(unsafe) static var elapsedTime: Double {
        getTimeSinceStartPtr()
    }

    public internal(set) nonisolated(unsafe) static var fixedDeltaTime: Double = 0
    public internal(set) nonisolated(unsafe) static var fixedTimeStep: Double = 0

    internal typealias TimeSinceStartup = @convention(c) () -> Double
    internal typealias DeltaTime = @convention(c) () -> Double

    internal nonisolated(unsafe) static var getTimeSinceStartPtr: TimeSinceStartup!
    internal nonisolated(unsafe) static var getDeltaTimePtr: DeltaTime!

    internal static nonisolated func setup() {
        getTimeSinceStartPtr = NativeLookupTable.getFunctionPointer(by: "Time_GetTimeSinceStart")
        getDeltaTimePtr = NativeLookupTable.getFunctionPointer(by: "Time_GetDeltaTime")
    }
}
