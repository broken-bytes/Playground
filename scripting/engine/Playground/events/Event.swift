enum EventType: Int32  {
    case health
    case system
    case input
    case network
}

struct Event {
    let type: EventType
}

enum SystemEventType: Int32 {
    case quit
    case windowResize
    case windowMove
    case windowFocus
    case windowLostFocus
}

struct SystemEvent {
    let type: EventType
    let systemType: SystemEventType
}
