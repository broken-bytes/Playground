@available(*, deprecated, message: "Never use this directly. Internal Engine Functionality")
public enum __Assembly {
    public static func registerComponent<T>(name: String, onSerialise: @escaping (T) -> String, onDeserialise: @escaping (String) -> T) {
        ComponentSerialisationManager.addComponent(name, onSerialise: onSerialise, onDeserialise: onDeserialise)
    }
}
