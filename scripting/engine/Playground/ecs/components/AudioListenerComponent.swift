@component
public struct AudioListenerComponent: Codable {
    var index: UInt8

    public init(index: UInt8) {
        self.index = index
    }
}
