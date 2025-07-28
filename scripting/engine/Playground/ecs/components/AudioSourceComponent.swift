#if os(Windows)
import WinSDK
#endif

public struct AudioSourceComponent {
    var previousPosition: Vector3 = .zero
    var forward: Vector3 = .zero
    var handle: UInt64 = UInt64.max
    var eventName: UnsafeMutablePointer<CChar>

    public init(with eventName: String) {
        let cStringLen = eventName.utf8CString.count
        self.eventName = UnsafeMutablePointer<CChar>.allocate(capacity: cStringLen)
        _ = eventName.withCString { basePtr in
            strcpy(self.eventName, basePtr)
        }

        // TODO: Fix possible leak here
    }
}
