
public enum Input {
    internal typealias GetAxis = @convention(c) (UnsafePointer<CChar>) -> Float
    internal typealias IsButtonDown = @convention(c) (UnsafePointer<CChar>) -> Bool
    internal typealias IsButtonPressed = @convention(c) (UnsafePointer<CChar>) -> Bool
    internal typealias IsButtonUp = @convention(c) (UnsafePointer<CChar>) -> Bool

    private static nonisolated(unsafe) var getAxisPtr: GetAxis!
    private static nonisolated(unsafe) var isButtonDownPtr: IsButtonDown!
    private static nonisolated(unsafe) var isButtonPressedPtr: IsButtonPressed!
    private static nonisolated(unsafe) var isButtonUpPtr: IsButtonUp!

    internal static nonisolated func start() {
        getAxisPtr = NativeLookupTable.getFunctionPointer(by: "Input_GetAxis")
        isButtonDownPtr = NativeLookupTable.getFunctionPointer(by: "Input_IsButtonPressed")
        isButtonPressedPtr = NativeLookupTable.getFunctionPointer(by: "Input_IsButtonDown")
        isButtonUpPtr = NativeLookupTable.getFunctionPointer(by: "Input_IsButtonUp")
    }

    public static nonisolated func getAxis(_ name: String) -> Float {
        return name.withCString { getAxisPtr($0) }
    }

    public static nonisolated func isButtonDown(_ name: String) -> Bool {
        return name.withCString { isButtonDownPtr($0) }
    }

    public static nonisolated func isButtonPressed(_ name: String) -> Bool {
        return name.withCString { isButtonPressedPtr($0) }
    }

    public static nonisolated func isButtonUp(_ name: String) -> Bool {
        return name.withCString { isButtonUpPtr($0) }
    }
}
