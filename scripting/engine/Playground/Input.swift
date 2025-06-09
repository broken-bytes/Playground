
public enum Input {
    internal typealias InputUpdate = @convention(c) () -> Void

    private static nonisolated(unsafe) var updatePtr: InputUpdate!

    internal static nonisolated func start() {
        updatePtr = NativeLookupTable.shared.getFunctionPointer(by: "Input_Update")
    }


    internal static nonisolated func update() {
        updatePtr()
    }
}
