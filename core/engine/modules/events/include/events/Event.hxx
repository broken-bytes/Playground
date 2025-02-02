#pragma once

#include "events/EventType.hxx"

namespace playground::events {
    class Event {
    public:
        EventType Type;

        Event(EventType type) : Type(type) {}
    };
}