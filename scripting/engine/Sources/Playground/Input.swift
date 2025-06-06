
public actor Input {
    internal typealias InputUpdate = @convention(c) () -> Void
    internal static let shared = Input()

    private nonisolated(unsafe) var updatePtr: InputUpdate!

    internal init() {

    }

    internal nonisolated func start() {
        updatePtr = NativeLookupTable.shared.getFunctionPointer(by: "Input_Update")
    }


    internal nonisolated func update() {
        updatePtr()
    }
}
