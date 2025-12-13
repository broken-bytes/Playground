#pragma once

#include <functional>

#include "events/EventType.hxx"
#include "events/Event.hxx"

namespace playground::events {
    auto Init() -> void;
	auto Subscribe(EventType type, std::function<void(Event*)> subscription) -> void;
	auto Emit(Event* event) -> void;
}