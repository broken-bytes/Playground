namespace Playground;

public class ComponentNotFoundException<T>(GameObject objc) : Exception($"{typeof(T)} not found on {objc.Name}") where T : Component;
public class ComponentAlreadyAttachedException<T>(GameObject objc) : Exception($"{typeof(T)} already present on {objc.Name}") where T : Component;
