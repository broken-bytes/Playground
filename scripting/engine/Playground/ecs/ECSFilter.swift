public struct ECSFilter {
    public enum Usage: Int16 {
        case read = 3
        case write = 4
        case readWrite = 5
    }

    public enum Operation: Int16 {
        case and = 0
        case or = 1
        case not = 2
    }

    internal let id: UInt64
    internal let usage: Int16
    internal let operation: Int16

    public init<T>(component: T.Type, usage: Usage, operation: Operation) {
        self.id = ECSHandler.id(for: T.self)
        self.usage = usage.rawValue
        self.operation = operation.rawValue
    }
}
